build_lib(
  LIBNAME innetwork-train
  SOURCE_FILES
    model/simple-MPI/base-protocol.cc
    model/simple-MPI/QUIC-protocol.cc
    utils/utils.cc
    utils/vectorop.cc
    model/simple-MPI/parameter.cc
    model/simple-MPI/circularbuffer.cc
    model/simple-MPI/simpleMPI.cc
    model/aggregator.cc
    model/consumer.cc
    model/producer.cc
  HEADER_FILES
    model/simple-MPI/base-protocol.h
    model/simple-MPI/QUIC-protocol.h
    utils/utils.h
    utils/vectorop.h
    model/simple-MPI/parameter.h
    model/simple-MPI/circularbuffer.h
    model/simple-MPI/simpleMPI.h
    model/aggregator.h
    model/consumer.h
    model/producer.h
  LIBRARIES_TO_LINK ${libinternet}
                    ${libapplications}
                    ${libquic}
)
