// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <mutex>
#include <regex>
#include <librealsense2/rs.hpp>
#include "../../third-party/realsense-file/rosbag/rosbag_storage/include/rosbag/bag.h"
#include "../../third-party/realsense-file/rosbag/rosbag_storage/include/rosbag/view.h"
#include "../../third-party/realsense-file/rosbag/msgs/sensor_msgs/Imu.h"
#include "../../third-party/realsense-file/rosbag/msgs/sensor_msgs/Image.h"
#include "../../third-party/realsense-file/rosbag/msgs/diagnostic_msgs/KeyValue.h"
#include "../../third-party/realsense-file/rosbag/msgs/std_msgs/UInt32.h"
#include "../../third-party/realsense-file/rosbag/msgs/std_msgs/String.h"
#include "../../third-party/realsense-file/rosbag/msgs/std_msgs/Float32.h"
#include "../../third-party/realsense-file/rosbag/msgs/realsense_msgs/StreamInfo.h"
#include "../../third-party/realsense-file/rosbag/msgs/realsense_msgs/ImuIntrinsic.h"
#include "../../third-party/realsense-file/rosbag/msgs/sensor_msgs/CameraInfo.h"
#include "../../third-party/realsense-file/rosbag/msgs/sensor_msgs/TimeReference.h"
#include "../../third-party/realsense-file/rosbag/msgs/geometry_msgs/Transform.h"

//#include "../../src/media/ros/ros_file_format.h"
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>
#include <noc_file_dialog.h>
#include "rendering.h"

using namespace std::chrono;
using namespace rosbag;
using namespace std;


std::string pretty_time(nanoseconds d)
{
    auto hhh = duration_cast<hours>(d);
    d -= hhh;
    auto mm = duration_cast<minutes>(d);
    d -= mm;
    auto ss = duration_cast<seconds>(d);
    d -= ss;
    auto ms = duration_cast<milliseconds>(d);

    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(3) << hhh.count() << ':' <<
        std::setfill('0') << std::setw(2) << mm.count() << ':' <<
        std::setfill('0') << std::setw(2) << ss.count() << '.' <<
        std::setfill('0') << std::setw(3) << ms.count();
    return stream.str();
}


std::ostream& operator<<(std::ostream& os, rosbag::compression::CompressionType c)
{
	switch (c)
	{
	case CompressionType::Uncompressed: os << "Uncompressed"; break;
	case CompressionType::BZ2: os << "BZ2"; break;
	case CompressionType::LZ4: os << "LZ4"; break;
	default: break;
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const rosbag::MessageInstance& m)
{
    if (m.isType<std_msgs::UInt32>())
    {
        os << "Value : " << m.instantiate<std_msgs::UInt32>()->data << std::endl;
    }
    if (m.isType<std_msgs::String>())
    {
        os << "Value : " << m.instantiate<std_msgs::String>()->data << std::endl;
    }
    if (m.isType<std_msgs::Float32>())
    {
        os << "Value : " << m.instantiate<std_msgs::Float32>()->data << std::endl;
    }
    if (m.isType<diagnostic_msgs::KeyValue>())
    {
        auto kvp = m.instantiate<diagnostic_msgs::KeyValue>();
        os << "Key   : " << kvp->key << std::endl;
        os << "Value : " << kvp->value << std::endl;
    }
    if (m.isType<realsense_msgs::StreamInfo>())
    {
        auto stream_info = m.instantiate<realsense_msgs::StreamInfo>();
        os << "Encoding       : " << stream_info->encoding << std::endl;
        os << "FPS            : " << stream_info->fps << std::endl;
        os << "Is Recommended : " << (stream_info->is_recommended ? "True" : "False") << std::endl;
    }
    if (m.isType<sensor_msgs::CameraInfo>())
    {
        auto camera_info = m.instantiate<sensor_msgs::CameraInfo>();
        os << "Width      : " << camera_info->width << std::endl;
        os << "Height     : " << camera_info->height << std::endl;
        os << "Intrinsics : " << std::endl;
        os << "  Focal Point      : " << camera_info->K[0] << ", " << camera_info->K[4] << std::endl;
        os << "  Principal Point  : " << camera_info->K[2] << ", " << camera_info->K[5] << std::endl;
        os << "  Coefficients     : "
           << camera_info->D[0] << ", "
           << camera_info->D[1] << ", "
           << camera_info->D[2] << ", "
           << camera_info->D[3] << ", "
           << camera_info->D[4] << std::endl;
        os << "  Distortion Model : " << camera_info->distortion_model << std::endl;

    }
    if (m.isType<realsense_msgs::ImuIntrinsic>())
    {
        
    }
    if (m.isType<sensor_msgs::Image>())
    {
        auto image = m.instantiate<sensor_msgs::Image>();
        os << "Encoding     : " << image->encoding << std::endl;
        os << "Width        : " << image->width << std::endl;
        os << "Height       : " << image->height << std::endl;
        os << "Step         : " << image->step << std::endl;
        os << "Frame Number : " << image->header.seq << std::endl;
        os << "Timestamp    : " << pretty_time(nanoseconds(image->header.stamp.toNSec())) << std::endl;
    }
    if (m.isType<sensor_msgs::Imu>())
    {
        
    }
    if (m.isType<sensor_msgs::TimeReference>())
    {
        auto tr = m.instantiate<sensor_msgs::TimeReference>();
        os << "  Header        : " << tr->header << std::endl;
        os << "  Source        : " << tr->source << std::endl;
        os << "  TimeReference : " << tr->time_ref << std::endl;
    }
    if (m.isType<geometry_msgs::Transform>())
    {
        os << "  Extrinsics : " << std::endl;
        auto tf = m.instantiate<geometry_msgs::Transform>();
        os << "    Rotation    : " << tf->rotation << std::endl;
        os << "    Translation : " << tf->translation << std::endl;
    }

    return os;
}

struct rosbag_content
{
    rosbag_content(const std::string& file)
    {
        bag.open(file);

        View entire_bag_view(bag);

        for (auto&& m : entire_bag_view)
        {
            topics_to_message_types[m.getTopic()].push_back(m.getDataType());
        }

        path = bag.getFileName();
        for (auto rit = path.rbegin(); rit != path.rend(); ++rit)
        {
            if (*rit == '\\' || *rit == '/')
                break;
            file_name += *rit;
        }
        std::reverse(file_name.begin(), file_name.end());

        version = rs2::to_string() << bag.getMajorVersion() << "." << bag.getMinorVersion();
        file_duration = get_duration(bag);
        size = 1.0 * bag.getSize() / (1024LL * 1024LL);
        compression_type = bag.getCompression();
    }

    rosbag_content(const rosbag_content& other)
    {
        bag.open(other.path);
        cache = other.cache;
        file_duration = other.file_duration;
        file_name = other.file_name;
        path = other.path;
        version = other.version;
        size = other.size;
        compression_type = other.compression_type;
        topics_to_message_types = other.topics_to_message_types;
    }
    rosbag_content(rosbag_content&& other)
    {
        other.bag.close();
        bag.open(other.path);
        cache = other.cache;
        file_duration = other.file_duration;
        file_name = other.file_name;
        path = other.path;
        version = other.version;
        size = other.size;
        compression_type = other.compression_type;
        topics_to_message_types = other.topics_to_message_types;

        other.cache.clear();
        other.file_duration = nanoseconds::zero();
        other.file_name.clear();
        other.path.clear();
        other.version.clear();
        other.size = 0;
        other.compression_type = static_cast<rosbag::compression::CompressionType>(0);
        other.topics_to_message_types.clear();
    }
    std::string instanciate_and_cache(const rosbag::MessageInstance& m)
    {
        auto key = std::make_tuple(m.getCallerId(), m.getDataType(), m.getMD5Sum(), m.getTopic(), m.getTime());
        if (cache.find(key) != cache.end())
        {
            return cache[key];
        }
        std::ostringstream oss;
        oss << m;
        cache[key] = oss.str();
        return oss.str();
    }

    nanoseconds get_duration(const Bag& bag)
    {
        View only_frames(bag, [](rosbag::ConnectionInfo const* info) {
            std::regex exp(R"RRR(/device_\d+/sensor_\d+/.*_\d+/(image|imu))RRR");
            return std::regex_search(info->topic, exp);
        });
        return nanoseconds((only_frames.getEndTime() - only_frames.getBeginTime()).toNSec());
    }

    std::map<std::tuple<std::string, std::string, std::string, std::string, ros::Time>, std::string> cache;
    nanoseconds file_duration;
    std::string file_name;
    std::string path;
    std::string version;
    double size;
    rosbag::compression::CompressionType compression_type;
    std::map<std::string, std::vector<std::string>> topics_to_message_types;
    Bag bag;
};

class Files
{
public:
    int size() const
    {
        return m_files.size();
    }

    rosbag_content& operator[](int index)
    {
        std::lock_guard<std::mutex> lock(mutex);
        return m_files[index];
    }

    void AddFiles(std::vector<std::string> const& files)
    {
        std::lock_guard<std::mutex> lock(mutex);

        for (auto&& file : files)
        {
            try
            {
                m_files.emplace_back(file);
            }
            catch (const std::exception& e)
            {
                last_error_message += e.what();
                last_error_message += "\n";
            }
        }
    }
    bool has_errors() const
    {
        return !last_error_message.empty();
    }
    std::string get_last_error()
    {
        return last_error_message;
    }
    void clear_errors()
    {
        last_error_message.clear();
    }
private:
    std::mutex mutex;
    std::vector<rosbag_content> m_files;
    std::string last_error_message;
};


int main(int argc, const char** argv)
{
    Files files;

    if (!glfwInit())
        return -1;
    
    auto window = glfwCreateWindow(1280, 720, "RealSense Rosbag Inspector", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, &files);
    glfwSetDropCallback(window, [](GLFWwindow* w, int count, const char** paths)
    {
        auto files = reinterpret_cast<Files*>(glfwGetWindowUserPointer(w));

        if (count <= 0)
            return;

        files->AddFiles(std::vector<std::string>(paths, paths + count));
    });

    ImGui_ImplGlfw_Init(window, true);

    std::map<std::string, int> num_topics_to_show;
    int w, h;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glfwGetWindowSize(window, &w, &h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        ImGui_ImplGlfw_NewFrame(1);

        auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoSavedSettings;

        ImGui::SetNextWindowSize({ 1.0f * w, 1.0f * h}, ImGuiSetCond_FirstUseEver);
        ImGui::SetNextWindowPos({ 0, 0 });
        ImGui::Begin("Rosbag Inspector", nullptr, flags);
        if (ImGui::Button(u8"Load File...", { 150, 0}))
        {
            auto ret = noc_file_dialog_open(NOC_FILE_DIALOG_OPEN, "ROS-bag\0*.bag\0", NULL, NULL);
            if (ret)
            {
                files.AddFiles({ ret });
            }
        }

        static int selected = 0;
        ImGui::BeginChild("loaded files", ImVec2(150, 0), true);
        for (int i = 0; i < files.size(); i++)
        {
            if (ImGui::Selectable(files[i].file_name.c_str(), selected == i))
            {
                selected = i;
                num_topics_to_show.clear();
            }
        }
        ImGui::EndChild();
        ImGui::SameLine();

        ImGui::BeginGroup();
        if (files.size() > 0)
        {
            auto& bag = files[selected];
            ImGui::BeginChild("Bag Content", ImVec2(0, 0), false);
            std::ostringstream oss;

            ImGui::Text("\t%s", std::string(rs2::to_string() << std::left << std::setw(20) << "Path: " << bag.path).c_str());
            ImGui::Text("\t%s", std::string(rs2::to_string() << std::left << std::setw(20) << "Bag Version: " << bag.version).c_str());
            ImGui::Text("\t%s", std::string(rs2::to_string() << std::left << std::setw(20) << "Duration: " << pretty_time(bag.file_duration)).c_str());
            ImGui::Text("\t%s", std::string(rs2::to_string() << std::left << std::setw(20) << "Size: " << bag.size << " MB").c_str());
            ImGui::Text("\t%s", std::string(rs2::to_string() << std::left << std::setw(20) << "Compression: " << bag.compression_type).c_str());

            if (ImGui::CollapsingHeader("Topics"))
            {
                for (auto&& topic_to_message_type : bag.topics_to_message_types)
                {
                    std::string topic = topic_to_message_type.first;
                    std::vector<std::string> messages_types = topic_to_message_type.second;
                    std::ostringstream oss;
                    oss << std::left << std::setw(100) << topic
                        << " " << std::left << std::setw(10) << messages_types.size() 
                        << std::setw(6) << std::string(" msg") + (messages_types.size() > 1 ? "s" : "")
                        << ": " << messages_types.front() << std::endl;
                    std::string line = oss.str();
                    auto pos = ImGui::GetCursorPos();
                    ImGui::SetCursorPos({ pos.x + 20, pos.y });
                    if (ImGui::CollapsingHeader(line.c_str()))
                    {
                        View messages(bag.bag, rosbag::TopicQuery(topic));
                        int count = 0;
                        num_topics_to_show[topic] = std::max(num_topics_to_show[topic], 10);
                        int max = num_topics_to_show[topic];
                        for (auto&& m : messages)
                        {
                            count++;
                            ImGui::Columns(2, "Message");
                            ImGui::Separator();
                            ImGui::Text("Timestamp"); ImGui::NextColumn();
                            ImGui::Text("Content"); ImGui::NextColumn();
                            ImGui::Separator();
                            ImGui::Text(pretty_time(nanoseconds(m.getTime().toNSec())).c_str()); ImGui::NextColumn();
                            ImGui::Text(bag.instanciate_and_cache(m).c_str());
                            ImGui::Columns(1);
                            ImGui::Separator();
                            if (count >= max)
                            {
                                int left = messages.size() - max;
                                if (left > 0)
                                {
                                    ImGui::Text("... %d more messages", left); 
                                    ImGui::SameLine();
                                    std::string label = rs2::to_string() << "Show More ##" << topic;
                                    if (ImGui::Button(label.c_str()))
                                    {
                                        num_topics_to_show[topic] += 10;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ImGui::EndChild();
        }
        ImGui::EndGroup();
        ImGui::End();

        if (files.has_errors())
        {
            ImGui::OpenPopup("Error");
            if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                std::string msg = rs2::to_string() << "Error: " << files.get_last_error();
                ImGui::Text(msg.c_str());
                ImGui::Separator();

                if (ImGui::Button("OK", ImVec2(120, 0))) 
                { 
                    ImGui::CloseCurrentPopup();
                    files.clear_errors();
                }
                ImGui::EndPopup();
            }
        }

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        glfwSwapBuffers(window);
        std::this_thread::sleep_for(milliseconds(10));
    }

    ImGui_ImplGlfw_Shutdown();
    glfwTerminate();
    return 0;
}

#ifdef WIN32
int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow

) {
    main(0, nullptr);
}
#endif