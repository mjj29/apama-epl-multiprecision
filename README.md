# apama-epl-multiprecision

Apama EPL plugin for extended-precision numbers

## Supported Apama version

This works with Apama 10.5.0.0 or later

## Building the plugin

To build the plugin run the following command from an Apama command prompt on Linux:

    mkdir -p $APAMA_WORK/lib $APAMA_WORK/monitors
    g++ -std=c++17 -o $APAMA_WORK/lib/libmultiprecision.so -I$APAMA\_HOME/include -L$APAMA_HOME/lib -lapclient -shared -fPIC plugin/multiprecision.cpp
    cp plugin/eventdefinitions/BigNumPlugin.mon $APAMA_WORK/monitors/BigNumPlugin.mon

On Windows run this command:

    g++ -std=c++17 -o %APAMA_WORK%\lib\libmultiprecision.so -I%APAMA_HOME%\include -L%APAMA_HOME%\lib -lapclient -shared -fPIC plugin\multiprecision.cpp
    copy plugin\eventdefinitions\BigNumPlugin.mon %APAMA_WORK%\monitors\BigNumPlugin.mon

To generate the Apama documentation for the RandomPlugin module run this command on Linux:

    java -jar $APAMA_HOME/lib/ap-generate-apamadoc.jar plugin/doc plugin/eventdefinitions

Or on Windows:

    java -jar %APAMA_HOME%\lib\ap-generate-apamadoc.jar plugin\doc plugin\eventdefinitions

## Building using Docker

There is a provided Dockerfile which will build the plugin, run tests and produce an image which is your base image, plus the CSV plugin. Application images can then be built from this image. To build the image run:

    docker build -t apama_with_bignum_plugin .

By default the public docker images from Docker Store for 10.5 will be used. To use another version run:

    docker build -t apama_with_bignum_plugin --build-arg APAMA_VERSION=10.7 .

To use custom images from your own repository then use:

    docker build -t apama_with_bignum_plugin --build-arg APAMA_BUILDER=builderimage --build-arg APAMA_IMAGE=runtimeimage .

## Running tests

To run the tests for the plugin you will need to use an Apama command prompt, then run the tests from within the tests directory:

    pysys run


## Using the plugin

To use the plugin first inject BigNumPlugin.mon, then ...

For more details on the available convenience functions, sources, distributions and outputs consult the API documentation for BigNumPlugin.

