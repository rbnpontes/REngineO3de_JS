/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include "AzToolsFramework_precompiled.h"

#include "ViewportUiWidgetCallbacks.h"

namespace AzToolsFramework::ViewportUi::Internal
{
    void ViewportUiWidgetCallbacks::AddWidget(
        QPointer<QObject> widget, const AZStd::function<void(QPointer<QObject>)>& updateCallback)
    {
        if (widget.isNull())
        {
            return;
        }

        // register and add the widget
        m_widgets.push_back(widget);

        // register the update callback if provided
        if (updateCallback)
        {
            m_updateCallbacks.insert({ widget, updateCallback });
        }
    }

    void ViewportUiWidgetCallbacks::RemoveWidget(QPointer<QObject> widget)
    {
        // deregister and remove the widget
        m_widgets.erase(AZStd::find(m_widgets.begin(), m_widgets.end(), widget));
    }

    void ViewportUiWidgetCallbacks::RegisterUpdateCallback(
        QPointer<QObject> widget, const AZStd::function<void(QPointer<QObject>)>& callback)
    {
        // if widget exists on the manager, register the callback
        auto callBackWidget = AZStd::find(m_widgets.begin(), m_widgets.end(), widget);
        AZ_Assert(callBackWidget != m_widgets.end(), "Unable to register a callback for an unregistered widget.")

        if (callBackWidget != m_widgets.end())
        {
            m_updateCallbacks.insert({ widget, callback });
        }
    }

    void ViewportUiWidgetCallbacks::Update()
    {
        // iterate through all the callbacks and call them with their respective widgets
        for (auto& widget : m_widgets)
        {
            if (widget.isNull())
            {
                RemoveWidget(widget);
            }
            // check if the widget has not been deleted externally
            else if (auto callback = m_updateCallbacks.find(widget);
                callback != m_updateCallbacks.end())
            {
                callback->second(widget);
            }
        }
    }
} // namespace AzToolsFramework::ViewportUi::Internal