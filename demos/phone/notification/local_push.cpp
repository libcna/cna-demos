// SPDX-License-Identifier: MS-PL
// Microsoft.Phone.Notification: a local push channel, raw/toast senders and caller-thread dispatch.
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Phone/Notification/HttpNotificationChannel.hpp"
#include "Microsoft/Phone/Notification/HttpNotificationEventArgs.hpp"
#include "Microsoft/Phone/Notification/NotificationChannelUriEventArgs.hpp"
#include "Microsoft/Phone/Notification/PushNotificationSender.hpp"

using namespace Microsoft::Phone::Notification;

namespace
{
    void LocalPush(CnaDemos::DemoContext& ctx)
    {
        constexpr const char* channelName = "CnaDemosLocalPush";
        HttpNotificationChannel channel(channelName, "CNA demos");

        ctx.Check(channel.getChannelNameProperty() == channelName &&
                      channel.getChannelUriProperty() == nullptr &&
                      !channel.getIsShellToastBoundProperty() &&
                      HttpNotificationChannel::Find(channelName) == nullptr,
                  "a closed channel exposes its name without a URI, binding or registry entry");

        int uriUpdates = 0;
        std::string announcedUri;
        channel.ChannelUriUpdated +=
            [&](System::Object*, const NotificationChannelUriEventArgs& args) {
                ++uriUpdates;
                announcedUri = args.getChannelUriProperty().ToString();
            };

        std::vector<std::vector<std::uint8_t>> bodies;
        std::vector<std::string> texts;
        std::vector<std::thread::id> handlerThreads;
        channel.HttpNotificationReceived +=
            [&](System::Object*, const HttpNotificationEventArgs& args) {
                bodies.push_back(args.getBodyProperty());
                texts.push_back(args.getBodyAsStringProperty());
                handlerThreads.push_back(std::this_thread::get_id());
            };

        channel.Open();
        const System::Uri* uri = channel.getChannelUriProperty();
        if (uri == nullptr)
        {
            ctx.Skip("the environment did not allow a loopback notification listener");
        }
        const std::string firstUri = uri->ToString();
        ctx.Check(uriUpdates == 1 && announcedUri == firstUri &&
                      firstUri.starts_with("http://127.0.0.1:") &&
                      firstUri.ends_with(std::string("/") + channelName + "/") &&
                      HttpNotificationChannel::Find(channelName) == &channel,
                  "Open announces and registers the exact loopback channel URI");

        channel.Open();
        ctx.Check(uriUpdates == 1 && channel.getChannelUriProperty()->ToString() == firstUri,
                  "opening an already-open channel is an idempotent no-op");

        channel.BindToShellToast();
        ctx.Check(channel.getIsShellToastBoundProperty(),
                  "BindToShellToast records the shell binding even without a phone shell");

        const std::string lowPayload = "raw-low: player joined";
        RawPushNotificationMessage low(MessageSendPriority::Low);
        low.RawData.assign(lowPayload.begin(), lowPayload.end());
        const bool lowAccepted = low.SendAsync(*channel.getChannelUriProperty());

        const std::string highPayload = "raw-high: turn ready";
        RawPushNotificationMessage high(MessageSendPriority::High);
        high.RawData.assign(highPayload.begin(), highPayload.end());
        const bool highAccepted = high.SendAsync(*channel.getChannelUriProperty());

        ToastPushNotificationMessage toast(MessageSendPriority::Normal);
        toast.Title = "Match ready";
        toast.SubTitle = "Tap to join";
        const bool toastAccepted = toast.SendAsync(*channel.getChannelUriProperty());

        ctx.Check(lowAccepted && highAccepted && toastAccepted && bodies.empty(),
                  "senders receive HTTP acceptance while notification handlers remain queued");

        const std::thread::id dispatchThread = std::this_thread::get_id();
        for (int attempt = 0; attempt < 100 && bodies.size() < 3; ++attempt)
        {
            channel.DispatchPendingNotificationsEXT();
            if (bodies.size() < 3)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        const std::string expectedToast =
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
            "<wp:Notification xmlns:wp=\"WPNotification\"><wp:Toast>"
            "<wp:Text1>Match ready</wp:Text1><wp:Text2>Tap to join</wp:Text2>"
            "</wp:Toast></wp:Notification>";
        ctx.Check(texts.size() == 3 && texts[0] == lowPayload && texts[1] == highPayload &&
                      texts[2] == expectedToast && bodies[0] == low.RawData &&
                      bodies[1] == high.RawData,
                  "raw bytes and the generated toast document arrive exactly in send order");
        ctx.Check(handlerThreads.size() == 3 &&
                      handlerThreads[0] == dispatchThread &&
                      handlerThreads[1] == dispatchThread &&
                      handlerThreads[2] == dispatchThread,
                  "DispatchPendingNotificationsEXT raises every handler on the caller thread");

        channel.Close();
        ctx.Check(channel.getChannelUriProperty() == nullptr &&
                      HttpNotificationChannel::Find(channelName) == nullptr,
                  "Close removes the URI and registry entry");

        channel.Close();
        channel.Open();
        ctx.Check(channel.getChannelUriProperty() != nullptr && uriUpdates == 2 &&
                      announcedUri == channel.getChannelUriProperty()->ToString() &&
                      HttpNotificationChannel::Find(channelName) == &channel,
                  "a closed channel can reopen and announces its new live URI once");
        channel.Close();
    }
}

CNA_DEMOS_CONSOLE(LocalPush, {
    .id = "phone.notification.local_push",
    .title = "Local raw and toast push notifications",
    .description = "Opens a loopback HttpNotificationChannel, discovers it by name, sends low/high raw and normal "
                   "toast notifications, then dispatches their exact bodies on the caller thread before closing "
                   "and reopening the channel.",
    .apis = {"HttpNotificationChannel::HttpNotificationChannel", "HttpNotificationChannel::Find",
             "HttpNotificationChannel::ChannelName", "HttpNotificationChannel::ChannelUri",
             "HttpNotificationChannel::IsShellToastBound", "HttpNotificationChannel::ChannelUriUpdated",
             "HttpNotificationChannel::HttpNotificationReceived", "HttpNotificationChannel::Open",
             "HttpNotificationChannel::Close", "HttpNotificationChannel::BindToShellToast",
             "HttpNotificationChannel::DispatchPendingNotificationsEXT",
             "NotificationChannelUriEventArgs::ChannelUri", "HttpNotificationEventArgs::Body",
             "HttpNotificationEventArgs::BodyAsString", "RawPushNotificationMessage::RawPushNotificationMessage",
             "RawPushNotificationMessage::RawData", "RawPushNotificationMessage::SendAsync",
             "ToastPushNotificationMessage::ToastPushNotificationMessage", "ToastPushNotificationMessage::Title",
             "ToastPushNotificationMessage::SubTitle", "ToastPushNotificationMessage::SendAsync",
             "MessageSendPriority::Low", "MessageSendPriority::Normal", "MessageSendPriority::High"},
    .tags = {"phone", "notifications", "push", "http", "loopback", "events"},
    .needs = CnaDemos::Need::Network,
    .restrictions = "Uses a loopback listener while the process is running. It does not demonstrate delivery to "
                    "an unaddressable/background application or a shell-rendered toast.",
})
