#include <opencv2/opencv.hpp>
#include <iostream>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

// To link Ws2_32.lib library
#pragma comment(lib, "Ws2_32.lib")

using namespace cv;
using namespace std;

// Global variables
bool drawing = false;
Point startPt(-1, -1);
Point endPt(-1, -1);
bool draw_mode = false;

// Sends coordinates via UDP using JSON format.
void sendCoordinates(int x_start, int y_start, int x_end, int y_end) {
    // Start Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cerr << "WSAStartup failed: " << iResult << endl;
        return;
    }
    
    // Create UDP socket
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        cerr << "Failed to create socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return;
    }
    
    // Set target server infos
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8554); // Port 8554
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    
    // Create coordinate string using JSON format
    ostringstream oss;
    oss << "{\"x_start\":" << x_start
        << ",\"y_start\":" << y_start
        << ",\"x_end\":" << x_end
        << ",\"y_end\":" << y_end << "}";
    string json_str = oss.str();
    
    // Send the message
    int sendResult = sendto(sock, json_str.c_str(), static_cast<int>(json_str.length()), 0,
                            (sockaddr*)&server_addr, sizeof(server_addr));
    if (sendResult == SOCKET_ERROR) {
        cerr << "sendto error: " << WSAGetLastError() << endl;
    }
    
    // Close the socket and clean Winsock
    closesocket(sock);
    WSACleanup();
}

// Mouse callback function
void draw_rectangle(int event, int x, int y, int flags, void* userdata) {
    if (draw_mode) {  // do conditions only if draw_mode is enable
        if (event == EVENT_LBUTTONDOWN) {
            drawing = true;
            startPt = Point(x, y);
            endPt = Point(x, y);
        } else if (event == EVENT_MOUSEMOVE) {
            if (drawing) {
                endPt = Point(x, y);
            }
        } else if (event == EVENT_LBUTTONUP) {
            drawing = false;
            endPt = Point(x, y);

            // Calculate the last coordinates
            int x_start = min(startPt.x, endPt.x);
            int y_start = min(startPt.y, endPt.y);
            int x_end = max(startPt.x, endPt.x);
            int y_end = max(startPt.y, endPt.y);

            // Send coordinates to server
            sendCoordinates(x_start, y_start, x_end, y_end);
        }
    }
}

int main() {
    // Catch RTSP stream
    VideoCapture cap("rtsp://localhost:8554/stream");
    if (!cap.isOpened()) {
        cerr << "Video stream could not opened!" << endl;
        return -1;
    }
    
    namedWindow("RTSP Stream with Bounding Box");
    setMouseCallback("RTSP Stream with Bounding Box", draw_rectangle);
    
    Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty())
            break;
        
        // Draw bounding box only if coordinates are valid
        if (startPt.x != -1 && startPt.y != -1 && endPt.x != -1 && endPt.y != -1) {
            int x_start = min(startPt.x, endPt.x);
            int y_start = min(startPt.y, endPt.y);
            int x_end = max(startPt.x, endPt.x);
            int y_end = max(startPt.y, endPt.y);
            rectangle(frame, Point(x_start, y_start), Point(x_end, y_end), Scalar(0, 0, 255), 2);
        }
        
        imshow("RTSP Stream with Bounding Box", frame);
        
        // Input control
        char key = (char)waitKey(1);
        if (key == 'q') {
            break;
        } else if (key == 'a') {
            draw_mode = !draw_mode;
        }
    }
    
    cap.release();
    destroyAllWindows();
    return 0;
}