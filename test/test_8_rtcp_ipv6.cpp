#include "test_common.hh"

constexpr char LOCAL_INTERFACE[] = "::1";
constexpr uint16_t LOCAL_PORT = 9200;

constexpr char REMOTE_ADDRESS[] = "::1";
constexpr uint16_t REMOTE_PORT = 9202;

constexpr uint16_t PAYLOAD_LEN = 256;
constexpr uint16_t FRAME_RATE = 30;
constexpr uint32_t EXAMPLE_RUN_TIME_S = 14;
constexpr int SEND_TEST_PACKETS = FRAME_RATE * EXAMPLE_RUN_TIME_S;
constexpr int PACKET_INTERVAL_MS = 1000 / FRAME_RATE;

void receiver_hook(uvgrtp::frame::rtcp_receiver_report* frame);
void sender_hook(uvgrtp::frame::rtcp_sender_report* frame);
void sdes_hook(uvgrtp::frame::rtcp_sdes_packet* frame);
void app_hook(uvgrtp::frame::rtcp_app_packet* frame);
void cleanup(uvgrtp::context& ctx, uvgrtp::session* local_session, uvgrtp::session* remote_session,
    uvgrtp::media_stream* send, uvgrtp::media_stream* receive);

TEST(RTCPTests_ip6, rtcp_ip6) {
    std::cout << "Starting uvgRTP IPv6 RTCP tests" << std::endl;

    // Creation of RTP stream. See sending example for more details
    uvgrtp::context ctx;
    uvgrtp::session* local_session = ctx.create_session(REMOTE_ADDRESS, LOCAL_INTERFACE);
    uvgrtp::session* remote_session = ctx.create_session(LOCAL_INTERFACE, REMOTE_ADDRESS);

    int flags = RCE_RTCP;

    uvgrtp::media_stream* local_stream = nullptr;
    if (local_session)
    {
        local_stream = local_session->create_stream(LOCAL_PORT, REMOTE_PORT, RTP_FORMAT_GENERIC, flags);
    }

    uvgrtp::media_stream* remote_stream = nullptr;
    if (remote_session)
    {
        remote_stream = remote_session->create_stream(REMOTE_PORT, LOCAL_PORT, RTP_FORMAT_GENERIC, flags);
    }

    EXPECT_NE(nullptr, remote_stream);

    if (local_stream)
    {
        EXPECT_EQ(RTP_OK, local_stream->get_rtcp()->install_receiver_hook(receiver_hook));
        EXPECT_EQ(RTP_OK, local_stream->get_rtcp()->install_sdes_hook(sdes_hook));
    }

    if (remote_stream)
    {
        EXPECT_EQ(RTP_OK, remote_stream->get_rtcp()->install_sender_hook(sender_hook));
        EXPECT_EQ(RTP_OK, remote_stream->get_rtcp()->install_sdes_hook(sdes_hook));
    }

    std::unique_ptr<uint8_t[]> test_frame = std::unique_ptr<uint8_t[]>(new uint8_t[PAYLOAD_LEN]);
    memset(test_frame.get(), 'b', PAYLOAD_LEN);
    send_packets(std::move(test_frame), PAYLOAD_LEN, local_session, local_stream, SEND_TEST_PACKETS, PACKET_INTERVAL_MS, true, RTP_NO_FLAGS);

    cleanup(ctx, local_session, remote_session, local_stream, remote_stream);
}

TEST(RTCP_reopen_receiver_ip6, rtcp_ip6) {
    std::cout << "Starting uvgRTP IPv6 RTCP reopen receiver test" << std::endl;

    // Creation of RTP stream. See sending example for more details
    uvgrtp::context ctx;
    uvgrtp::session* local_session = ctx.create_session(REMOTE_ADDRESS);
    uvgrtp::session* remote_session = ctx.create_session(LOCAL_INTERFACE);

    int flags = RCE_RTCP;

    uvgrtp::media_stream* local_stream = nullptr;
    if (local_session)
    {
        local_stream = local_session->create_stream(LOCAL_PORT, REMOTE_PORT, RTP_FORMAT_GENERIC, flags);
    }

    uvgrtp::media_stream* remote_stream = nullptr;
    if (remote_session)
    {
        remote_stream = remote_session->create_stream(REMOTE_PORT, LOCAL_PORT, RTP_FORMAT_GENERIC, flags);
    }

    EXPECT_NE(nullptr, remote_stream);

    if (local_stream)
    {
        EXPECT_EQ(RTP_OK, local_stream->get_rtcp()->install_receiver_hook(receiver_hook));
        EXPECT_EQ(RTP_OK, local_stream->get_rtcp()->install_sdes_hook(sdes_hook));
    }

    if (remote_stream)
    {
        EXPECT_EQ(RTP_OK, remote_stream->get_rtcp()->install_sender_hook(sender_hook));
        EXPECT_EQ(RTP_OK, remote_stream->get_rtcp()->install_sdes_hook(sdes_hook));
    }

    if (local_stream)
    {
        std::unique_ptr<uint8_t[]> test_frame = std::unique_ptr<uint8_t[]>(new uint8_t[PAYLOAD_LEN]);
        memset(test_frame.get(), 'b', PAYLOAD_LEN);
        send_packets(std::move(test_frame), PAYLOAD_LEN, local_session, local_stream, SEND_TEST_PACKETS / 2,
            PACKET_INTERVAL_MS, true, RTP_NO_FLAGS);

        if (remote_stream)
        {
            std::cout << "Closing and reopening receiver for testing purposes" << std::endl;
            remote_session->destroy_stream(remote_stream);
            remote_stream = remote_session->create_stream(REMOTE_PORT, LOCAL_PORT, RTP_FORMAT_GENERIC, flags);
            EXPECT_NE(nullptr, remote_stream);
        }

        test_frame = std::unique_ptr<uint8_t[]>(new uint8_t[PAYLOAD_LEN]);
        memset(test_frame.get(), 'b', PAYLOAD_LEN);
        send_packets(std::move(test_frame), PAYLOAD_LEN, local_session, local_stream, SEND_TEST_PACKETS / 2,
            PACKET_INTERVAL_MS, true, RTP_NO_FLAGS);
    }

    cleanup(ctx, local_session, remote_session, local_stream, remote_stream);
}

TEST(RTCP_double_bind_test_ip6, rtcp_ip6) {
    // Here we test if there are errors when double binding RTCP into the same port

    std::cout << "Starting uvgRTP IPv6 RTCP double bind tests" << std::endl;

    // Creation of RTP stream. See sending example for more details
    uvgrtp::context ctx;
    uvgrtp::session* local_session = ctx.create_session(REMOTE_ADDRESS);
    uvgrtp::session* remote_session = ctx.create_session(LOCAL_INTERFACE);

    int flags = RCE_RTCP;

    uvgrtp::media_stream* local_stream = nullptr;
    if (local_session)
    {
        local_stream = local_session->create_stream(LOCAL_PORT, REMOTE_PORT, RTP_FORMAT_GENERIC, flags);
    }

    uvgrtp::media_stream* remote_stream = nullptr;
    if (remote_session)
    {
        // this is invalid since the ports are the same
        remote_stream = remote_session->create_stream(LOCAL_PORT, REMOTE_PORT, RTP_FORMAT_GENERIC, flags);
    }

    EXPECT_EQ(nullptr, remote_stream);

    cleanup(ctx, local_session, remote_session, local_stream, remote_stream);
}