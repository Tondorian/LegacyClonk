/*
 * LegacyClonk
 *
 * Copyright (c) 2022, The LegacyClonk Team and contributors
 *
 * Distributed under the terms of the ISC license; see accompanying file
 * "COPYING" for details.
 *
 * "Clonk" is a registered trademark of Matthes Bender, used with permission.
 * See accompanying file "TRADEMARK" for details.
 *
 * To redistribute this file separately, substitute the full license texts
 * for the above references.
 */

#include "C4ToastWinRT.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Data.Xml.Dom.h>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Data::Xml::Dom;
using namespace winrt::Windows::UI::Notifications;

C4ToastSystemWinRT::C4ToastSystemWinRT()
{
	MapHResultError(&ToastNotificationManager::GetDefault);
}

C4ToastImplWinRT::C4ToastImplWinRT()
	: notification{MapHResultError(&ToastNotificationManager::GetTemplateContent, ToastTemplateType::ToastText02)},
	  notifier{MapHResultError([] { return ToastNotificationManager::CreateToastNotifier(_CRT_WIDE(STD_APPUSERMODELID)); })},
	  eventHandler{std::move(MapHResultError([this] { return winrt::make_self<EventHandler>(notification, C4ToastImpl::eventHandler); }))}
{
}

void C4ToastImplWinRT::AddAction(std::string_view action)
{
	MapHResultError([&, this]
	{
		const auto content = notification.Content();
		const auto actions = content.GetElementsByTagName(L"actions");

		IXmlNode actionNode;
		if (actions.Length())
		{
			actionNode = actions.Item(0);
		}
		else
		{
			const auto toast = content.GetElementsByTagName(L"toast");
			assert(toast.Length());

			const auto first = toast.Item(0).as<IXmlElement>();
			first.SetAttribute(L"template", L"ToastGeneric");
			first.SetAttribute(L"duration", L"long");
			const auto actionElement = content.CreateElement(L"actions");
			first.AppendChild(actionElement);

			actionNode = actionElement.as<IXmlNode>();
		}

		const auto actionElement = content.CreateElement(L"action");

		const auto actionWide = StdStringEncodingConverter::WinAcpToUtf16(action);
		actionElement.SetAttribute(L"content", actionWide);
		actionElement.SetAttribute(L"arguments", actionWide);

		actionNode.AppendChild(actionElement);
	});
}

void C4ToastImplWinRT::SetExpiration(const std::uint32_t expiration)
{
	MapHResultError([expiration, this] { notification.ExpirationTime(winrt::clock::now() + std::chrono::milliseconds{expiration}); });
}

void C4ToastImplWinRT::SetText(std::string_view text)
{
	MapHResultError(&C4ToastImplWinRT::SetTextNode, this, 1, StdStringEncodingConverter::WinAcpToUtf16(text));
}

void C4ToastImplWinRT::SetTitle(std::string_view title)
{
	MapHResultError(&C4ToastImplWinRT::SetTextNode, this, 0, StdStringEncodingConverter::WinAcpToUtf16(title));
}

void C4ToastImplWinRT::Show()
{
	MapHResultError(&ToastNotifier::Show, notifier, notification);
}

void C4ToastImplWinRT::Hide()
{
	MapHResultError(&ToastNotifier::Hide, notifier, notification);
}

void C4ToastImplWinRT::SetTextNode(const std::uint32_t index, std::wstring &&text)
{
	const auto item = notification.Content().GetElementsByTagName(L"text").Item(index);
	item.InnerText(std::move(text));
}

C4ToastImplWinRT::EventHandler::EventHandler(ToastNotification &notification, C4ToastEventHandler * const &eventHandler)
	 : eventHandler{eventHandler}
{
	notification.Activated([weak{get_weak()}](const ToastNotification &sender, const IInspectable &args)
	{
		if (auto that{weak.get()}; that && that->eventHandler)
		{
			if (const auto activatedEventArgs = args.try_as<IToastActivatedEventArgs>(); activatedEventArgs)
			{
				that->eventHandler->OnAction(StdStringEncodingConverter{}.Utf16ToWinAcp(activatedEventArgs.Arguments().c_str()));
			}
			else
			{
				that->eventHandler->Activated();
			}
		}
	});

	notification.Dismissed([weak{get_weak()}](const ToastNotification &sender, const ToastDismissedEventArgs &args)
	{
		if (auto that = weak.get(); that && that->eventHandler)
		{
			that->eventHandler->Dismissed();
		}
	});

	notification.Failed([weak{get_weak()}](const ToastNotification &sender, const ToastFailedEventArgs &args)
	{
		if (auto that = weak.get(); that && that->eventHandler)
		{
			that->eventHandler->Failed();
		}
	});
}

#if NTDDI_VERSION < NTDDI_WIN8
extern "C" void WINAPI Clonk_GetSystemTimePreciseAsFileTime(LPFILETIME lpSystemTimeAsFileTime) noexcept
{
	*lpSystemTimeAsFileTime = winrt::file_time{static_cast<std::uint64_t>(std::chrono::file_clock::now().time_since_epoch().count())};
}
#endif
