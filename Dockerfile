ARG APAMA_VERSION=10.5
ARG APAMA_BUILDER=store/softwareag/apama-builder:${APAMA_VERSION}
ARG APAMA_IMAGE=store/softwareag/apama-correlator:${APAMA_VERSION}

FROM ${APAMA_BUILDER} as builder

FROM ubuntu:latest as compiler

COPY --from=builder /opt/softwareag /opt/softwareag

ENV \
	APAMA_HOME=/opt/softwareag/Apama \
	APAMA_WORK=/apama_work
ENV \
   PATH=${APAMA_HOME}/bin:${PATH} \
	LD_LIBRARY_PATH=${APAMA_HOME}/lib:${APAMA_WORK}/lib \
	PYTHONPATH=${APAMA_HOME}/third_party/python/lib/python3.6/site-packages:${LD_LIBRARY_PATH}

RUN apt-get update && apt-get install -y g++-8 python3 libboost-dev

COPY . $APAMA_WORK/plugin

RUN mkdir -p $APAMA_WORK/lib $APAMA_WORK/monitors
RUN g++-8 -std=c++17 -o $APAMA_WORK/lib/libmultiprecision.so -I$APAMA_HOME/include -L$APAMA_HOME/lib -lapclient -I$APAMA_WORK/plugin/plugin -shared -fPIC $APAMA_WORK/plugin/plugin/multiprecision.cpp
RUN cp $APAMA_WORK/plugin/plugin/eventdefinitions/RandomPlugin.mon $APAMA_WORK/monitors/

RUN cd ${APAMA_WORK}/plugin/tests && pysys run | tee logfile && grep 'THERE WERE NO NON PASSES' logfile

FROM ubuntu:latest

RUN apt update && apt -y install libboost && apt clean

ENV \
	APAMA_HOME=/opt/softwareag/Apama \
	APAMA_WORK=/apama_work
ENV \
	PATH=${APAMA_HOME}/bin \
	LD_LIBRARY_PATH=${APAMA_HOME}/lib:${APAMA_WORK}/lib \
	PYTHONPATH=${APAMA_HOME}/third_party/python/lib/python3.6/site-packages

COPY --from=builder  /opt/softwareag /opt/softwareag
COPY --from=compiler ${APAMA_WORK}/lib ${APAMA_WORK}/lib

WORKDIR ${APAMA_WORK}
CMD ["correlator"]

