/*
 *  Copyright (c) 2016 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <limits.h>

#include "test/codec_factory.h"
#include "test/encode_test_driver.h"
#include "test/util.h"
#include "test/video_source.h"
#include "third_party/googletest/src/include/gtest/gtest.h"

namespace {

const int kVideoSourceWidth = 320;
const int kVideoSourceHeight = 240;
const int kFramesToEncode = 2;

class RealtimeTest
    : public ::libvpx_test::EncoderTest,
      public ::libvpx_test::CodecTestWithParam<libvpx_test::TestMode> {
 protected:
  RealtimeTest() : EncoderTest(GET_PARAM(0)), frame_packets_(0) {}
  virtual ~RealtimeTest() {}

  virtual void SetUp() {
    InitializeConfig();
    cfg_.g_lag_in_frames = 0;
    SetMode(::libvpx_test::kRealTime);
  }

  virtual void BeginPassHook(unsigned int /*pass*/) {
    // TODO(tomfinegan): We're changing the pass value here to make sure
    // we get frames when real time mode is combined with |g_pass| set to
    // VPX_RC_FIRST_PASS. This is necessary because EncoderTest::RunLoop() sets
    // the pass value based on the mode passed into EncoderTest::SetMode(),
    // which overrides the one specified in SetUp() above.
    cfg_.g_pass = VPX_RC_FIRST_PASS;
  }
  virtual void FramePktHook(const vpx_codec_cx_pkt_t * /*pkt*/) {
    frame_packets_++;
  }

  bool IsVP9() const {
#if CONFIG_VP9_ENCODER
    return codec_ == &libvpx_test::kVP9;
#else
    return false;
#endif
  }

  void TestIntegerOverflow(unsigned int width, unsigned int height) {
    ::libvpx_test::RandomVideoSource video;
    video.SetSize(width, height);
    video.set_limit(20);
    cfg_.rc_target_bitrate = UINT_MAX;
    ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
  }

  int frame_packets_;
};

TEST_P(RealtimeTest, RealtimeFirstPassProducesFrames) {
  ::libvpx_test::RandomVideoSource video;
  video.SetSize(kVideoSourceWidth, kVideoSourceHeight);
  video.set_limit(kFramesToEncode);
  ASSERT_NO_FATAL_FAILURE(RunLoop(&video));
  EXPECT_EQ(kFramesToEncode, frame_packets_);
}

TEST_P(RealtimeTest, IntegerOverflow) {
  if (IsVP9()) {
    // TODO(https://crbug.com/webm/1749): This should match VP8.
    TestIntegerOverflow(800, 480);
  } else {
    TestIntegerOverflow(2048, 2048);
  }
}

TEST_P(RealtimeTest, IntegerOverflowLarge) {
  if (IsVP9()) {
    GTEST_SKIP() << "TODO(https://crbug.com/webm/1750): Enable this test after "
                    "undefined sanitizer warnings are fixed.";
    // TestIntegerOverflow(16384, 16384);
  } else {
    GTEST_SKIP()
        << "TODO(https://crbug.com/webm/1748,https://crbug.com/webm/1751):"
        << " Enable this test after bitstream errors & undefined sanitizer "
           "warnings are fixed.";
    // TestIntegerOverflow(16383, 16383);
  }
}

VP8_INSTANTIATE_TEST_CASE(RealtimeTest,
                          ::testing::Values(::libvpx_test::kRealTime));
VP9_INSTANTIATE_TEST_CASE(RealtimeTest,
                          ::testing::Values(::libvpx_test::kRealTime));

}  // namespace
