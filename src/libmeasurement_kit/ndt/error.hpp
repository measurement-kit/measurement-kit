// Part of Measurement Kit <https://measurement-kit.github.io/>.
// Measurement Kit is free software under the BSD license. See AUTHORS
// and LICENSE for more information on the copying conditions.
#ifndef SRC_LIBMEASUREMENT_KIT_NDT_ERROR_HPP
#define SRC_LIBMEASUREMENT_KIT_NDT_ERROR_HPP

#include <measurement_kit/common.hpp>

namespace mk {
namespace ndt {

MK_DEFINE_ERR(MK_ERR_NDT( 0), ReadingMessageTypeLengthError, "ndt_cannot_read_type_length")
MK_DEFINE_ERR(MK_ERR_NDT( 1), ReadingMessagePayloadError, "ndt_cannot_read_payload")
MK_DEFINE_ERR(MK_ERR_NDT( 2), MessageTooLongError, "ndt_message_too_long")
MK_DEFINE_ERR(MK_ERR_NDT( 3), ConnectControlConnectionError, "ndt_cannot_establish_control_connection")
MK_DEFINE_ERR(MK_ERR_NDT( 4), FormatExtendedLoginMessageError, "ndt_cannot_format_extended_login_message")
MK_DEFINE_ERR(MK_ERR_NDT( 5), WriteExtendedLoginMessageError, "ndt_cannot_write_extended_login_message")
MK_DEFINE_ERR(MK_ERR_NDT( 6), ReadingKickoffMessageError, "ndt_cannot_read_kickoff_message")
MK_DEFINE_ERR(MK_ERR_NDT( 7), InvalidKickoffMessageError, "ndt_invalid_kickoff_message")
MK_DEFINE_ERR(MK_ERR_NDT( 8), ReadingSrvQueueMessageError, "ndt_cannot_read_srv_queue_message")
MK_DEFINE_ERR(MK_ERR_NDT( 9), NotSrvQueueMessageError, "ndt_missing_expected_srv_queue_message")
MK_DEFINE_ERR(MK_ERR_NDT(10), InvalidSrvQueueMessageError, "ndt_invalid_srv_queue_message")
MK_DEFINE_ERR(MK_ERR_NDT(11), QueueServerFaultError, "ndt_server_queue_fault")
MK_DEFINE_ERR(MK_ERR_NDT(12), ReadingServerVersionMessageError, "ndt_cannot_read_server_version")
MK_DEFINE_ERR(MK_ERR_NDT(13), NotServerVersionMessageError, "ndt_missing_expected_server_version")
MK_DEFINE_ERR(MK_ERR_NDT(14), ReadingTestsIdMessageError, "ndt_cannot_read_tests_id")
MK_DEFINE_ERR(MK_ERR_NDT(15), NotTestsIdMessageError, "ndt_missing_expected_tests_id_message")
MK_DEFINE_ERR(MK_ERR_NDT(16), InvalidTestIdError, "ndt_invalid_tests_id_message")
MK_DEFINE_ERR(MK_ERR_NDT(17), UnknownTestIdError, "ndt_received_unknown_test_id")
MK_DEFINE_ERR(MK_ERR_NDT(18), TestFailedError, "ndt_test_failed")
MK_DEFINE_ERR(MK_ERR_NDT(19), ReadingResultsOrLogoutError, "ndt_cannot_read_results_or_logout_message")
MK_DEFINE_ERR(MK_ERR_NDT(20), NotResultsOrLogoutError, "ndt_missing_expected_results_or_logout_message")
MK_DEFINE_ERR(MK_ERR_NDT(21), WaitingCloseError, "ndt_error_while_waiting_for_close_message")
MK_DEFINE_ERR(MK_ERR_NDT(22), DataAfterLogoutError, "ndt_received_data_after_logout_message")
MK_DEFINE_ERR(MK_ERR_NDT(23), InvalidPortError, "ndt_received_invalid_port")
MK_DEFINE_ERR(MK_ERR_NDT(24), MlabnsQueryError, "ndt_error_querying_mlabns")
MK_DEFINE_ERR(MK_ERR_NDT(25), ReadingTestPrepareError, "ndt_cannot_read_test_prepare_message")
MK_DEFINE_ERR(MK_ERR_NDT(26), NotTestPrepareError, "ndt_missing_expected_test_prepare_message")
MK_DEFINE_ERR(MK_ERR_NDT(27), ConnectTestConnectionError, "ndt_cannot_establish_testing_connection")
MK_DEFINE_ERR(MK_ERR_NDT(28), ReadingTestStartError, "ndt_cannot_read_test_start_message")
MK_DEFINE_ERR(MK_ERR_NDT(29), NotTestStartError, "ndt_missing_expected_test_start_error")
MK_DEFINE_ERR(MK_ERR_NDT(30), ReadingTestMsgError, "ndt_cannot_read_test_msg_message")
MK_DEFINE_ERR(MK_ERR_NDT(31), NotTestMsgError, "ndt_missing_expected_test_msg")
MK_DEFINE_ERR(MK_ERR_NDT(32), ReadingTestFinalizeError, "ndt_cannot_read_test_finalize_message")
MK_DEFINE_ERR(MK_ERR_NDT(33), NotTestFinalizeError, "ndt_missing_expected_test_finalize_message")
MK_DEFINE_ERR(MK_ERR_NDT(34), SerializingClientVersionError, "ndt_cannot_serialize_client_version")
MK_DEFINE_ERR(MK_ERR_NDT(35), SerializingClientApplicationError, "ndt_cannot_serialize_client_application")
MK_DEFINE_ERR(MK_ERR_NDT(36), SerializingFinalMetaError, "ndt_cannot_serialize_final_meta_message")
MK_DEFINE_ERR(MK_ERR_NDT(37), WritingMetaError, "ndt_cannot_write_meta_message")
MK_DEFINE_ERR(MK_ERR_NDT(38), SerializingTestMsgError, "ndt_cannot_serialize_test_msg_message")
MK_DEFINE_ERR(MK_ERR_NDT(39), WritingTestMsgError, "ndt_cannot_write_test_msg_message")
MK_DEFINE_ERR(MK_ERR_NDT(40), QueueServerBusyError, "ndt_server_busy")
MK_DEFINE_ERR(MK_ERR_NDT(41), FormatMsgWaitingError, "ndt_cannot_format_msg_waiting_message")
MK_DEFINE_ERR(MK_ERR_NDT(42), InvalidDurationError, "ndt_invalid_duration_setting")
MK_DEFINE_ERR(MK_ERR_NDT(43), InvalidSnapsDelayError, "ndt_invalid_snaps_delay_setting")
MK_DEFINE_ERR(MK_ERR_NDT(44), InvalidNumStreamsError, "ndt_invalid_num_streams_setting")

} // namespace ndt
} // namespace mk
#endif
