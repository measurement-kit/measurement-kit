// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NDT_NDT_HPP
#define MEASUREMENT_KIT_NDT_NDT_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace ndt {

void run_with_specific_server(std::string address, int port,
                              Callback<Error> callback, Settings settings = {},
                              Var<Logger> logger = Logger::global(),
                              Var<Reactor> reactor = Reactor::global());

void run(Callback<Error> callback, Settings settings = {},
         Var<Logger> logger = Logger::global(),
         Var<Reactor> reactor = Reactor::global());

MK_DECLARE_TEST_DSL(NdtTest)

MK_DEFINE_ERR(6000, ReadingMessageTypeLengthError, "unknown_error 6000")
MK_DEFINE_ERR(6001, ReadingMessageTypeError, "unknown_error 6001")
MK_DEFINE_ERR(6002, ReadingMessageLengthError, "unknown_error 6002")
MK_DEFINE_ERR(6003, ReadingMessageBodyError, "unknown_error 6003")
MK_DEFINE_ERR(6004, MessageTooLongError, "unknown_error 6004")
MK_DEFINE_ERR(6005, ConnectControlConnectionError, "unknown_error 6005")
MK_DEFINE_ERR(6006, FormatExtendedLoginMessageError, "unknown_error 6006")
MK_DEFINE_ERR(6007, WriteExtendedLoginMessageError, "unknown_error 6007")
MK_DEFINE_ERR(6008, ReadingKickoffMessageError, "unknown_error 6008")
MK_DEFINE_ERR(6009, InvalidKickoffMessageError, "unknown_error 6009")
MK_DEFINE_ERR(6010, ReadingSrvQueueMessageError, "unknown_error 6010")
MK_DEFINE_ERR(6011, NotSrvQueueMessageError, "unknown_error 6011")
MK_DEFINE_ERR(6012, InvalidSrvQueueMessageError, "unknown_error 6012")
MK_DEFINE_ERR(6013, UnhandledSrvQueueMessageError, "unknown_error 6013")
MK_DEFINE_ERR(6014, ReadingServerVersionMessageError, "unknown_error 6014")
MK_DEFINE_ERR(6015, NotServerVersionMessageError, "unknown_error 6015")
MK_DEFINE_ERR(6016, ReadingTestsIdMessageError, "unknown_error 6016")
MK_DEFINE_ERR(6017, NotTestsIdMessageError, "unknown_error 6017")
MK_DEFINE_ERR(6018, InvalidTestIdError, "unknown_error 6018")
MK_DEFINE_ERR(6019, UnknownTestIdError, "unknown_error 6019")
MK_DEFINE_ERR(6020, TestFailedError, "unknown_error 6020")
MK_DEFINE_ERR(6021, ReadingResultsOrLogoutError, "unknown_error 6021")
MK_DEFINE_ERR(6022, NotResultsOrLogoutError, "unknown_error 6022")
MK_DEFINE_ERR(6023, WaitingCloseError, "unknown_error 6023")
MK_DEFINE_ERR(6024, DataAfterLogoutError, "unknown_error 6024")
MK_DEFINE_ERR(6025, InvalidPortError, "unknown_error 6025")
MK_DEFINE_ERR(6026, MlabnsQueryError, "unknown_error 6026")
MK_DEFINE_ERR(6027, ReadingTestPrepareError, "unknown_error 6027")
MK_DEFINE_ERR(6028, NotTestPrepareError, "unknown_error 6028")
MK_DEFINE_ERR(6029, ConnectTestConnectionError, "unknown_error 6029")
MK_DEFINE_ERR(6030, ReadingTestStartError, "unknown_error 6030")
MK_DEFINE_ERR(6031, NotTestStartError, "unknown_error 6031")
MK_DEFINE_ERR(6032, ReadingTestMsgError, "unknown_error 6032")
MK_DEFINE_ERR(6033, NotTestMsgError, "unknown_error 6033")
MK_DEFINE_ERR(6034, ReadingTestFinalizeError, "unknown_error 6034")
MK_DEFINE_ERR(6035, NotTestFinalizeError, "unknown_error 6035")
MK_DEFINE_ERR(6036, SerializingClientVersionError, "unknown_error 6036")
MK_DEFINE_ERR(6037, SerializingClientApplicationError, "unknown_error 6037")
MK_DEFINE_ERR(6038, SerializingFinalMetaError, "unknown_error 6038")
MK_DEFINE_ERR(6039, WritingMetaError, "unknown_error 6039")
MK_DEFINE_ERR(6040, SerializingTestMsgError, "unknown_error 6040")
MK_DEFINE_ERR(6041, WritingTestMsgError, "unknown_error 6041")

} // namespace ndt
} // namespace mk
#endif
