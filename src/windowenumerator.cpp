#include "windowenumerator.h"

#include "wlr-foreign-toplevel-management-unstable-v1-client-protocol.h"

#include <QVariantMap>

#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

#include <wayland-client.h>

namespace {
struct WindowRecord {
    zwlr_foreign_toplevel_handle_v1 *handle = nullptr;
    QString appId;
    QString title;
    bool closed = false;
};

struct Context {
    wl_display *display = nullptr;
    wl_registry *registry = nullptr;
    zwlr_foreign_toplevel_manager_v1 *manager = nullptr;
    std::vector<std::unique_ptr<WindowRecord>> windows;
};

void handleTitle(void *data, zwlr_foreign_toplevel_handle_v1 *, const char *title)
{
    static_cast<WindowRecord *>(data)->title = QString::fromUtf8(title ? title : "");
}

void handleAppId(void *data, zwlr_foreign_toplevel_handle_v1 *, const char *appId)
{
    static_cast<WindowRecord *>(data)->appId = QString::fromUtf8(appId ? appId : "");
}

void handleOutputEnter(void *, zwlr_foreign_toplevel_handle_v1 *, wl_output *) {}
void handleOutputLeave(void *, zwlr_foreign_toplevel_handle_v1 *, wl_output *) {}
void handleState(void *, zwlr_foreign_toplevel_handle_v1 *, wl_array *) {}
void handleDone(void *, zwlr_foreign_toplevel_handle_v1 *) {}

void handleClosed(void *data, zwlr_foreign_toplevel_handle_v1 *)
{
    static_cast<WindowRecord *>(data)->closed = true;
}

void handleParent(void *, zwlr_foreign_toplevel_handle_v1 *,
                  zwlr_foreign_toplevel_handle_v1 *) {}

const zwlr_foreign_toplevel_handle_v1_listener handleListener = {
    handleTitle,
    handleAppId,
    handleOutputEnter,
    handleOutputLeave,
    handleState,
    handleDone,
    handleClosed,
    handleParent,
};

void managerToplevel(void *data, zwlr_foreign_toplevel_manager_v1 *,
                     zwlr_foreign_toplevel_handle_v1 *handle)
{
    auto *context = static_cast<Context *>(data);
    auto window = std::make_unique<WindowRecord>();
    window->handle = handle;
    zwlr_foreign_toplevel_handle_v1_add_listener(handle, &handleListener, window.get());
    context->windows.push_back(std::move(window));
}

void managerFinished(void *, zwlr_foreign_toplevel_manager_v1 *) {}

const zwlr_foreign_toplevel_manager_v1_listener managerListener = {
    managerToplevel,
    managerFinished,
};

void registryGlobal(void *data, wl_registry *registry, uint32_t name,
                    const char *interface, uint32_t version)
{
    auto *context = static_cast<Context *>(data);
    if (std::strcmp(interface, zwlr_foreign_toplevel_manager_v1_interface.name) == 0) {
        context->manager = static_cast<zwlr_foreign_toplevel_manager_v1 *>(
            wl_registry_bind(registry, name,
                             &zwlr_foreign_toplevel_manager_v1_interface,
                             std::min(version, 3u)));
        zwlr_foreign_toplevel_manager_v1_add_listener(
            context->manager, &managerListener, context);
    }
}

void registryGlobalRemove(void *, wl_registry *, uint32_t) {}

const wl_registry_listener registryListener = {
    registryGlobal,
    registryGlobalRemove,
};
}

QVariantList enumerateOpenWindows(QString *error)
{
    Context context;
    context.display = wl_display_connect(nullptr);
    if (!context.display) {
        if (error) *error = QStringLiteral("Could not connect to the active Wayland display.");
        return {};
    }

    context.registry = wl_display_get_registry(context.display);
    wl_registry_add_listener(context.registry, &registryListener, &context);
    if (wl_display_roundtrip(context.display) < 0 || !context.manager) {
        if (error) *error = QStringLiteral("River does not expose the foreign-toplevel list protocol.");
        wl_registry_destroy(context.registry);
        wl_display_disconnect(context.display);
        return {};
    }

    if (wl_display_roundtrip(context.display) < 0) {
        if (error) *error = QStringLiteral("Could not read River's open-window list.");
    }

    QVariantList result;
    for (const auto &window : context.windows) {
        if (!window->closed) {
            result.append(QVariantMap{{QStringLiteral("appId"), window->appId},
                                      {QStringLiteral("title"), window->title}});
        }
        zwlr_foreign_toplevel_handle_v1_destroy(window->handle);
    }
    zwlr_foreign_toplevel_manager_v1_destroy(context.manager);
    wl_registry_destroy(context.registry);
    wl_display_flush(context.display);
    wl_display_disconnect(context.display);

    std::sort(result.begin(), result.end(), [](const QVariant &left, const QVariant &right) {
        return left.toMap().value(QStringLiteral("appId")).toString()
            < right.toMap().value(QStringLiteral("appId")).toString();
    });
    return result;
}
