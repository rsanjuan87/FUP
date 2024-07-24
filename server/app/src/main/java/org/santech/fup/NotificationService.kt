package org.santech.fup

import android.content.Context
import android.service.notification.NotificationListenerService
import android.service.notification.StatusBarNotification

class NotificationService : NotificationListenerService() {
    private val TAG: String = "NOTIFICATION_CHANNEL"
    private lateinit var context: Context

    override fun onCreate() {
        super.onCreate()
        context = applicationContext
    }

    override fun onNotificationPosted(sbn: StatusBarNotification) {
        MsgHelper.printNotif(context, TAG, "posted", sbn)
    }

    override fun onNotificationRemoved(sbn: StatusBarNotification) {
        MsgHelper.printNotif(context, TAG, "removed", sbn)
        try {
            updater!!.updateApp(sbn.packageName, null)
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    fun setListener(updater: UpdaterInterface?) {
        Companion.updater = updater
    }

    companion object {
        var updater: UpdaterInterface? = null
    }


}


