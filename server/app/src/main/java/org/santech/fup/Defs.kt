package org.santech.fup

import android.content.Context
import java.io.File

class Defs {
    companion object {
        val KEY_NOTIFICACTION_CHANNEL = "NOTIFICATION_CHANNEL"
        val KEY_PACKAGE_ID = "org.santech.notifyme"
        val KEY_GET_PACKAGES = "fup:packages"
        val KEY_GET_LAUNCHERS = "fup:get_launchers"
        val KEY_GET_PACKAGE_INFO = "fup:packages_info"
        val KEY_GET_PACKAGE_ICON = "fup:packages_icon"
        val KEY_SAVED_PACKAGE_ICON = "fup:icon_saved"
        val KEY_RUN_PACKAGE = "fup:run_package"
        val KEY_START_SERVICE = "fup:start_service"
        val KEY_STOP_SERVICE = "fup:stop_service"

        val KEY_APP_RESUMED = "fup:app_resumed"

        val KEY_LAUCH_ACTIVITY = "fup:launch_activity"

        val KEY_GET_NOTIFICATIONS_STATUS = "fup:get_notifications_status"
        val KEY_GET_REQUEST_NOTIFICATIONS_ACCESS = "fup:request_notifications_access"


        val KEY_FUP_DIR_TMP = "/data/local/tmp/fup"

        fun KEY_FUP_ICONS_DIR(context: Context) : File {
            return File(context.filesDir, "icons")
        }

        fun KEY_FUP_DIR(context: Context) : File {
            return File(context.filesDir.path)
        }
    }
}
