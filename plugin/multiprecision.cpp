
#include <epl_plugin.hpp>
#include <stdexcept>
#include <type_traits>
#include <iostream>

#include <boost/multiprecision/gmp.hpp>
#include <boost/multiprecision/cpp_int.hpp>

using com::apama::epl::EPLPlugin;
using com::apama::epl::data_t;
using com::apama::epl::custom_t;
using com::apama::epl::get;
using com::apama::epl::apply_visitor;
using com::apama::epl::visitor;

namespace com::apamax::multiprecision {

class multiprecision_error: public std::runtime_error
{
	using std::runtime_error::runtime_error;
};


/** Random plugin class */
class Multiprecision: public EPLPlugin<Multiprecision>
{
private:
	struct MultiprecisionBase
	{
		enum {
			ARBITRARY=1,
			FIXED_128=128,
			FIXED_256=256,
			FIXED_512=512,
			FIXED_1024=1024
		} type_t;
		virtual void add(const data_t &o) = 0;
		virtual void sub(const data_t &o) = 0;
		virtual void set(const data_t &o) = 0;
		virtual void mod(const data_t &o) = 0;
		virtual void pow(const data_t &o) = 0;
		virtual void mul(const data_t &o) = 0;
		virtual void div(const data_t &o) = 0;
		virtual void abs() = 0;
		virtual int64_t comp(const data_t &o) = 0;
		virtual int64_t mag() = 0;
		virtual std::string toString() = 0;
		virtual double toFloat() = 0;
		virtual int64_t toInteger(int64_t offset) = 0;
		virtual custom_t<MultiprecisionBase> clone() = 0;
		virtual type_t type() = 0;
	};

	struct setOp { template<typename A, typename B> void operator()(A&a, B&b) { a.assign(b); } };
	struct subOp { template<typename A, typename B> void operator()(A&a, B&b) { a -= b; } };
	struct addOp { template<typename A, typename B> void operator()(A&a, B&b) { a += b; } };
	struct modOp { template<typename A, typename B> void operator()(A&a, B&b) { a %= b; } };
	struct mulOp { template<typename A, typename B> void operator()(A&a, B&b) { a *= b; } };
	struct divOp { template<typename A, typename B> void operator()(A&a, B&b) { a /= b; } };
	struct powOp { template<typename A, typename B> void operator()(A&a, B&b) { a = pow(a, b); } };
	struct compOp { template<typename A, typename B> int64_t operator()(A&a, B&b) { return a.compare(b); } };

	template<typename T, typename OP, typename RV>
	class biopVisitor: public const_visitor<biopVisitor<T, OP, RV>, RV>
	{
		biopVisitor(T &t): t{t} {}
		T &t;
		RV visitInteger(int64_t i) const { return OP()(t, i); }
		RV visitCustom(const sag_underlying_custom_t &i) const;
	};

	template<type_t TYPE, typename IMPL>
	struct MultiprecisionBaseImpl
	{
		IMPL data;
		MultiprecisionBaseImpl(const data_t &initial) { set(initial); }
		type_t type() override { return TYPE; }
		void set(const data_t &o) override {
			if (DATA_STRING == o.type()) {
				data.assign(get<const char*>(o));
			} else {
				apply_visitor(biopVisitor<IMPL, setOp, void>(data), o);
			}
		}

		void add(const data_t &o) override { apply_visitor(biopVisitor<IMPL, addOp, void>(data), o); }
		void sub(const data_t &o) override { apply_visitor(biopVisitor<IMPL, subOp, void>(data), o); }
		void mod(const data_t &o) override { apply_visitor(biopVisitor<IMPL, modOp, void>(data), o); }
		void pow(const data_t &o) override { apply_visitor(biopVisitor<IMPL, powOp, void>(data), o); }
		void mul(const data_t &o) override { apply_visitor(biopVisitor<IMPL, mulOp, void>(data), o); }
		void div(const data_t &o) override { apply_visitor(biopVisitor<IMPL, divOp, void>(data), o); }
		void abs() override { data = abs(data); }
		int64_t comp(const data_t &o) override { apply_visitor(biopVisitor<IMPL, compOp, int64_t>(data), o); }
		std::string toString() override { return data.str(); }
		double toFloat() override { return data.convert_t<double>(); }
		int64_t toInteger(int64_t offset) override
		{
			IMPL tmp = data;
			tmp <<= offset;
			return tmp.convert_to<int64_t>();
		}
		custom_t<MultiprecisionBase> clone() override;
	};

	template<type_t SIZE>
	using FixedPrecision = public MultiprecisionBaseImpl<SIZE, boost::multiprecision::number<cpp_int_backend<SIZE, SIZE, signed_magnitude, checked, void> > >;

	using ArbitraryPrecision = public MultiprecisionBaseImpl<MultiprecisionBase::ARBITRARY, boost::multiprecision::mpz_int>;

public:
	Multiprecision(): base_plugin_t("Multiprecision") {}
	/** expose the methods to EPL */
	static void initialize(base_plugin_t::method_data_t &md)
	{
		md.registerMethod<decltype(&Multiprecision::createFixedPrecision), &Multiprecision::createFixedPrecision>("createFixedPrecision", "action<integer, any> returns chunk");
		md.registerMethod<decltype(&Multiprecision::createArbitraryPrecision), &Multiprecision::createArbitraryPrecision>("createArbitraryPrecision", "action<integer, any> returns chunk");
		md.registerMethod<decltype(&Multiprecision::sub), &Multiprecision::sub>("sub", "action<chunk, any>");
		md.registerMethod<decltype(&Multiprecision::set), &Multiprecision::set>("set", "action<chunk, any>");
		md.registerMethod<decltype(&Multiprecision::mod), &Multiprecision::mod>("mod", "action<chunk, any>");
		md.registerMethod<decltype(&Multiprecision::pow), &Multiprecision::pow>("pow", "action<chunk, any>");
		md.registerMethod<decltype(&Multiprecision::mul), &Multiprecision::mul>("mul", "action<chunk, any>");
		md.registerMethod<decltype(&Multiprecision::div), &Multiprecision::div>("div", "action<chunk, any>");
		md.registerMethod<decltype(&Multiprecision::comp), &Multiprecision::comp>("comp", "action<chunk, any> returns integer");
		md.registerMethod<decltype(&Multiprecision::abs), &Multiprecision::abs>("abs");
		md.registerMethod<decltype(&Multiprecision::toString), &Multiprecision::toString>("toString");
		md.registerMethod<decltype(&Multiprecision::toFloat), &Multiprecision::toFloat>("toFloat");
		md.registerMethod<decltype(&Multiprecision::toInteger), &Multiprecision::toInteger>("toInteger");
		md.registerMethod<decltype(&Multiprecision::toFixed), &Multiprecision::toFixed>("toFixed", "action<chunk, integer> returns com.apamax.bignum.FixedPrecisionInt");
		md.registerMethod<decltype(&Multiprecision::toArbitrary), &Multiprecision::toArbitrary>("toArbitrary", "action<chunk> returns com.apamax.bignum.ArbitraryPrecisionInt");
		md.registerMethod<decltype(&Multiprecision::clone), &Multiprecision::clone>("clone");
	}
private:


	custom_t<MultiprecisionBase> createFixedPrecision(int64_t precision, const data_t &initial)
	{
		switch (precision) {
			case 128: {
				custom_t<FixedPrecision<MultiprecisionBase::FIXED_128>> val{new FixedPrecision<MultiprecisionBase::FIXED_128>(initial)};
				return std::move(reinterpret_cast<custom_t<MultiprecisionBase>&>(val));
			}
			case 256: {
				custom_t<FixedPrecision<MultiprecisionBase::FIXED_256>> val{new FixedPrecision<MultiprecisionBase::FIXED_256>(initial)};
				return std::move(reinterpret_cast<custom_t<MultiprecisionBase>&>(val));
			}
			case 512: {
				custom_t<FixedPrecision<MultiprecisionBase::FIXED_512>> val{new FixedPrecision<MultiprecisionBase::FIXED_512>(initial)};
				return std::move(reinterpret_cast<custom_t<MultiprecisionBase>&>(val));
			}
			case 1024: {
				custom_t<FixedPrecision<MultiprecisionBase::FIXED_1024>> val{new FixedPrecision<MultiprecisionBase::FIXED_1024>(initial)};
				return std::move(reinterpret_cast<custom_t<MultiprecisionBase>&>(val));
			}
			default: throw multiprecision_error("Invalid fixed precision, must be one of 128, 156, 512 or 1024");
		}
	}
	custom_t<MultiprecisionBase> createArbitraryPrecision(const data_t &initial)
	{
		custom_t<ArbitraryPrecision> val{new ArbitraryPrecision(initial)};
		return std::move(reinterpret_cast<custom_t<MultiprecisionBase>&>(val));
	}

	void add(const custom_t<MultiprecisionBase> &v, const data_t &o) { v->add(o); }
	void sub(const custom_t<MultiprecisionBase> &v, const data_t &o) { v->sub(o); }
	void set(const custom_t<MultiprecisionBase> &v, const data_t &o) { v->set(o); }
	void mod(const custom_t<MultiprecisionBase> &v, const data_t &o) { v->mod(o); }
	void pow(const custom_t<MultiprecisionBase> &v, const data_t &o) { v->pow(o); }
	void mul(const custom_t<MultiprecisionBase> &v, const data_t &o) { v->mul(o); }
	void div(const custom_t<MultiprecisionBase> &v, const data_t &o) { v->div(o); }
	int64_t comp(const custom_t<MultiprecisionBase> &v, const data_t &o) { return v->comp(o); }
	void abs(const custom_t<MultiprecisionBase> &v) { v->abs(); }
	std::string toString(const custom_t<MultiprecisionBase> &v) { return v->toString(); }
	double toFloat(const custom_t<MultiprecisionBase> &v) { return v->toFloat(); }
	double toInteger(const custom_t<MultiprecisionBase> &v, int64_t offset) { return v->toInteger(offset); }
	data_t toFixed(const custom_t<MultiprecisionBase> &v, int64_t size) {}
	data_t toArbitrary(const custom_t<MultiprecisionBase> &v) {}
	custom_t<MultiprecisionBase> clone(const custom_t<MultiprecisionBase> &v) { return v->clone(); }
};

template<typename T, typename OP, typename RV>
RV Multiprecision::biopVisitor<T, OP, RV>::visitCustom(const sag_underlying_custom_t &i) const;
{
	const custom_t<MultiprecisionBase> &base = reinterpret_cast<const custom_t<MultiprecisionBase> &>(i);
	switch (base->type()) {
		case MultiprecisionBase::ARBITRARY:
			return OP()(t, static_cast<const ArbitraryPrecision&>(*base).data);
		case MultiprecisionBase::FIXED_128:
			return OP()(t, static_cast<const FixedPrecision<MultiprecisionBase::FIXED_128&>(*base).data);
		case MultiprecisionBase::FIXED_256:
			return OP()(t, static_cast<const FixedPrecision<MultiprecisionBase::FIXED_256&>(*base).data);
		case MultiprecisionBase::FIXED_512:
			return OP()(t, static_cast<const FixedPrecision<MultiprecisionBase::FIXED_512&>(*base).data);
		case MultiprecisionBase::FIXED_1024:
			return OP()(t, static_cast<const FixedPrecision<MultiprecisionBase::FIXED_1024&>(*base).data);
		default:
				assert(false); // shouldn't happen
				throw multiprecision_error("Unknown error");
	}
}

template<type_t TYPE, typename IMPL>
custom_t<Multiprecision::MultiprecisionBase> Multiprecision::MultiprecisionBaseImpl::clone()
{
	if constexpr (TYPE == MultiprecisionBase::ARBITRARY) {
		custom_t<ArbitraryPrecision> val{new ArbitraryPrecision(data)};
		return std::move(reinterpret_cast<custom_t<MultiprecisionBase>&>(val));
	} else {
		custom_t<FixedPrecision<TYPE>> val{new FixedPrecision<TYPE>(data)};
		return std::move(reinterpret_cast<custom_t<MultiprecisionBase>&>(val));
	}
}


APAMA_DECLARE_EPL_PLUGIN(Multiprecision)

} // com::apamax::multiprecision
