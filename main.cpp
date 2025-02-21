// Author : Serhat KILIÇ
// Task: Create a gstreamer server that retrieves video stream from camera/file using RTSP Server

//Importing libraries
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

// if GST_X264_ENC_TUNE_ZERO_LATENCY variable isn't defined it sets it to 1
#ifndef GST_X264_ENC_TUNE_ZERO_LATENCY
#define GST_X264_ENC_TUNE_ZERO_LATENCY 1
#endif

// Define our static function it takes GstElement, GstPad and gpointer
static void on_pad_added(GstElement *element, GstPad *pad, gpointer data) {
    // It takes pads caps and if doesn't take it creates a err message
    GstCaps *caps = gst_pad_get_current_caps(pad);
    if (!caps) {
        g_printerr("Failed to get caps from pad.\n");
        return;
    }

    //Creating structure and getting name
    GstStructure *structure = gst_caps_get_structure(caps, 0);
    const gchar *name = gst_structure_get_name(structure);

    // Link only "video/x-h264" pads
    if (g_str_has_prefix(name, "video/x-h264")) {
        // Getting pad and if linking is fail it prints err
        GstPad *sink_pad = gst_element_get_static_pad(GST_ELEMENT(data), "sink");
        if (gst_pad_link(pad, sink_pad) != GST_PAD_LINK_OK) {
            g_printerr("Failed to link demuxer and parser.\n");
        }
        // Freeing reference and caps
        gst_object_unref(sink_pad);
    }
    gst_caps_unref(caps);
}

int main(int argc, char *argv[]) {
    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create pipeline
    GstElement *pipeline = gst_pipeline_new("media-server");
    GstElement *src, *demuxer, *video_parser, *decoder, *encoder, *payloader, *sink;

    // 1. File Source (filesrc)
    src = gst_element_factory_make("filesrc", "file-source");
    g_object_set(src, "location", "test.mp4", NULL);

    // 2. Demuxer (qtdemux for MP4 file)
    demuxer = gst_element_factory_make("qtdemux", "demuxer");

    // 3. Video Parser (h264parse for H264)
    video_parser = gst_element_factory_make("h264parse", "video-parser");

    // 4. Video Decoder (avdec_h264 for H264)
    decoder = gst_element_factory_make("avdec_h264", "decoder");

    // 5. Video Encoder (x264enc for H264)
    encoder = gst_element_factory_make("x264enc", "encoder");
    g_object_set(encoder, "tune", GST_X264_ENC_TUNE_ZERO_LATENCY, NULL);

    // 6. RTP Payloader (rtph264pay)
    payloader = gst_element_factory_make("rtph264pay", "payloader");
    g_object_set(payloader, "config-interval", -1, "pt", 96, NULL);

    // 7. UDP Sink (for RTSP server)
    sink = gst_element_factory_make("udpsink", "sink");
    g_object_set(sink, "host", "127.0.0.1", "port", 5000, NULL);

    // Add elements to pipeline
    gst_bin_add_many(GST_BIN(pipeline), src, demuxer, video_parser, decoder, encoder, payloader, sink, NULL);

    // Link source and demuxer
    if (!gst_element_link(src, demuxer)) {
        g_printerr("Failed to link source and demuxer.\n");
        return -1;
    }

    // Connect demuxer pad-added signal to link with video parser
    g_signal_connect(demuxer, "pad-added", G_CALLBACK(on_pad_added), video_parser);

    // Link remaining elements
    if (!gst_element_link_many(video_parser, decoder, encoder, payloader, sink, NULL)) {
        g_printerr("Failed to link pipeline elements.\n");
        return -1;
    }

    // RTSP Server Setup
    GstRTSPServer *server = gst_rtsp_server_new();
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();

    // Set launch pipeline for RTSP server; adjust caps as necessary
    gst_rtsp_media_factory_set_launch(factory,
        "( udpsrc port=5000 caps=\"application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96\" ! "
        "rtph264depay ! h264parse ! rtph264pay name=pay0 pt=96 )");
    
    // RTSP mount point
    gst_rtsp_mount_points_add_factory(mounts, "/stream", factory);
    gst_rtsp_server_attach(server, NULL);

    // Start pipeline
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_print("RTSP Stream: rtsp://localhost:8554/stream\n");

    // Main loop
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    return 0;
}
