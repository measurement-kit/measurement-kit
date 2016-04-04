// Part of measurement-kit <https://measurement-kit.github.io/>.
// Measurement-kit is free software. See AUTHORS and LICENSE for more
// information on the copying conditions.
#ifndef SRC_NDT_DEFINITIONS_HPP
#define SRC_NDT_DEFINITIONS_HPP

// TODO: add here the definition of all messages
#define COMM_FAILURE 0
#define SRV_QUEUE 1
#define MSG_LOGIN 2
#define TEST_PREPARE 3
#define TEST_START 4
#define TEST_MSG 5
#define TEST_FINALIZE 6
#define MSG_ERROR 7
#define MSG_RESULTS 8
#define MSG_LOGOUT 9
#define MSG_WAITING 10
#define MSG_EXTENDED_LOGIN 11

// TODO: add here the definition of all tests
#define TEST_NONE 0
#define TEST_MID 1 << 0
#define TEST_C2S 1 << 1
#define TEST_S2C 1 << 2
#define TEST_SFW 1 << 3
#define TEST_STATUS 1 << 4
#define TEST_META 1 << 5

#define KICKOFF_MESSAGE "123456 654321"
#define KICKOFF_MESSAGE_SIZE (sizeof (KICKOFF_MESSAGE) -1)

#define MSG_NDT_VERSION "v3.7.0" ///< Version of NDT that we declare to be

#endif
