// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef MEASUREMENT_KIT_NDT_ERROR_HPP
#define MEASUREMENT_KIT_NDT_ERROR_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace ndt {

MK_DEFINE_ERR(MK_ERR_NDT( 0), ReadingMessageTypeLengthError, "")
MK_DEFINE_ERR(MK_ERR_NDT( 1), ReadingMessagePayloadError, "")
MK_DEFINE_ERR(MK_ERR_NDT( 2), MessageTooLongError, "")
MK_DEFINE_ERR(MK_ERR_NDT( 3), ConnectControlConnectionError, "")
MK_DEFINE_ERR(MK_ERR_NDT( 4), FormatExtendedLoginMessageError, "")
MK_DEFINE_ERR(MK_ERR_NDT( 5), WriteExtendedLoginMessageError, "")
MK_DEFINE_ERR(MK_ERR_NDT( 6), ReadingKickoffMessageError, "")
MK_DEFINE_ERR(MK_ERR_NDT( 7), InvalidKickoffMessageError, "")
MK_DEFINE_ERR(MK_ERR_NDT( 8), ReadingSrvQueueMessageError, "")
MK_DEFINE_ERR(MK_ERR_NDT( 9), NotSrvQueueMessageError, "")
MK_DEFINE_ERR(MK_ERR_NDT(10), InvalidSrvQueueMessageError, "")
MK_DEFINE_ERR(MK_ERR_NDT(11), UnhandledSrvQueueMessageError, "")
MK_DEFINE_ERR(MK_ERR_NDT(12), ReadingServerVersionMessageError, "")
MK_DEFINE_ERR(MK_ERR_NDT(13), NotServerVersionMessageError, "")
MK_DEFINE_ERR(MK_ERR_NDT(14), ReadingTestsIdMessageError, "")
MK_DEFINE_ERR(MK_ERR_NDT(15), NotTestsIdMessageError, "")
MK_DEFINE_ERR(MK_ERR_NDT(16), InvalidTestIdError, "")
MK_DEFINE_ERR(MK_ERR_NDT(17), UnknownTestIdError, "")
MK_DEFINE_ERR(MK_ERR_NDT(18), TestFailedError, "")
MK_DEFINE_ERR(MK_ERR_NDT(19), ReadingResultsOrLogoutError, "")
MK_DEFINE_ERR(MK_ERR_NDT(20), NotResultsOrLogoutError, "")
MK_DEFINE_ERR(MK_ERR_NDT(21), WaitingCloseError, "")
MK_DEFINE_ERR(MK_ERR_NDT(22), DataAfterLogoutError, "")
MK_DEFINE_ERR(MK_ERR_NDT(23), InvalidPortError, "")
MK_DEFINE_ERR(MK_ERR_NDT(24), MlabnsQueryError, "")
MK_DEFINE_ERR(MK_ERR_NDT(25), ReadingTestPrepareError, "")
MK_DEFINE_ERR(MK_ERR_NDT(26), NotTestPrepareError, "")
MK_DEFINE_ERR(MK_ERR_NDT(27), ConnectTestConnectionError, "")
MK_DEFINE_ERR(MK_ERR_NDT(28), ReadingTestStartError, "")
MK_DEFINE_ERR(MK_ERR_NDT(29), NotTestStartError, "")
MK_DEFINE_ERR(MK_ERR_NDT(30), ReadingTestMsgError, "")
MK_DEFINE_ERR(MK_ERR_NDT(31), NotTestMsgError, "")
MK_DEFINE_ERR(MK_ERR_NDT(32), ReadingTestFinalizeError, "")
MK_DEFINE_ERR(MK_ERR_NDT(33), NotTestFinalizeError, "")
MK_DEFINE_ERR(MK_ERR_NDT(34), SerializingClientVersionError, "")
MK_DEFINE_ERR(MK_ERR_NDT(35), SerializingClientApplicationError, "")
MK_DEFINE_ERR(MK_ERR_NDT(36), SerializingFinalMetaError, "")
MK_DEFINE_ERR(MK_ERR_NDT(37), WritingMetaError, "")
MK_DEFINE_ERR(MK_ERR_NDT(38), SerializingTestMsgError, "")
MK_DEFINE_ERR(MK_ERR_NDT(39), WritingTestMsgError, "")

} // namespace ndt
} // namespace mk
#endif
