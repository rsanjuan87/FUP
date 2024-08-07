package org.santech.fup

import android.content.BroadcastReceiver
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.provider.Settings
import android.util.Log
import java.io.File
import java.io.FileOutputStream


class BroadcastReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent) {
        try {
            val data = intent.getStringExtra("action")
            when (data) {
                Defs.KEY_START_SERVICE -> {
                    context.startService(Intent(context, NotificationService::class.java))
                }

                Defs.KEY_STOP_SERVICE -> {
                    context.startService(Intent(context, NotificationService::class.java))
                }

                Defs.KEY_GET_LAUNCHERS -> {
                    val pm = context.packageManager
                    val l = mutableListOf<String>()
                    val list = MsgHelper.getLauncherApps(context)
                    for (i in list) {
                        var s = "{"
//                        val actyName = "${i.activityInfo.packageName}-${i.activityInfo.name}"
                        val pkg = i.activityInfo.packageName
                        val activity = i.activityInfo.targetActivity
                        MsgHelper.saveIcon(context, pkg, i.loadIcon(pm))
                        s += "\"LAUNCHER_LABEL\": \"${i.loadLabel(pm)}\","
                        s += "\"LAUNCHER_ACTY\": \"${activity}\","
                        s += "\"LAUNCHER_PKG\": \"${pkg}\""
                        s += "}"
                        l.add(s)
                    }
                    val text = "[" + l.joinToString(",") + "]"

                    val dir = Defs.KEY_FUP_DIR(context)
                    val file = File(dir, "launchers.json")
                    try {

                        var fos: FileOutputStream? = null
                        if (file.exists()) {
                            file.delete()
                        }
                        file.createNewFile()
                        fos = FileOutputStream(file)
                        fos.write(text.toByteArray())
                        fos.close()
                    } catch (e: Exception) {
                        e.printStackTrace()
                    }
                    MsgHelper.print(context, Defs.KEY_GET_LAUNCHERS, file.path)
                }

                Defs.KEY_GET_PACKAGES -> {

                }

                Defs.KEY_GET_PACKAGE_INFO -> {
                    val pkg = intent.getStringExtra("package")!!
                    val pm = context.packageManager
                    val appInfo = pm.getApplicationInfo(pkg, 0)
                    val appLabel = pm.getApplicationLabel(appInfo).toString()
                    val appIcon = pm.getApplicationIcon(appInfo)
                    var s = "{"
                    s += "\"APP_LABEL\": \"${appLabel}\","
                    s += "\"APP_PKG\": \"${pkg}\""
                    s += "}"
                    MsgHelper.print(context, Defs.KEY_GET_PACKAGE_INFO, s)
                }

                Defs.KEY_LAUCH_ACTIVITY -> {
                    val pm = context.packageManager
                    val pkg = intent.getStringExtra("package")!!
                    val appInfo = pm.getApplicationInfo(pkg, 0)
                    val appIcon = pm.getApplicationIcon(appInfo)
                    NotificationService.updater?.updateApp(pkg, appIcon)
//                    val appLabel = pm.getApplicationLabel(appInfo).toString()
                    var acty = intent.getStringExtra("activity")!!
                    val i = Intent()
                    if (acty.isEmpty() || acty == "null") {
                        i.action = Intent.ACTION_MAIN
                        i.addCategory(Intent.CATEGORY_LAUNCHER)
                        acty = pm.getLaunchIntentForPackage(pkg)?.component?.className!!
                    }
                    i.component = ComponentName(pkg, acty)
                    i.flags = Intent.FLAG_ACTIVITY_NEW_TASK
                    context.startActivity(i)

                }

                Defs.KEY_GET_NOTIFICATIONS_STATUS -> {
                    val status = isNotificationServiceEnabled(context)
                    MsgHelper.print(context, Defs.KEY_GET_NOTIFICATIONS_STATUS, status.toString())
                }

                Defs.KEY_GET_REQUEST_NOTIFICATIONS_ACCESS -> {
                    openNotificationAccessSettings(context)
                }

            }




            Log.d("broadcast", data ?: "done");
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    private fun isNotificationServiceEnabled(context: Context): Boolean {
        val pkgName = context.packageName
        val flat = Settings.Secure.getString(context.contentResolver, "enabled_notification_listeners")
        if (!flat.isNullOrEmpty()) {
            val names = flat.split(":").toTypedArray()
            for (name in names) {
                val componentName = ComponentName.unflattenFromString(name)
                if (componentName != null && componentName.packageName == pkgName) {
                    return true
                }
            }
        }
        return false
    }

    fun openNotificationAccessSettings(context: Context) {
        // El servicio de escucha de notificaciones no está habilitado, redirigir para habilitar
        val intent = Intent(Settings.ACTION_NOTIFICATION_LISTENER_SETTINGS)
        context.startActivity(intent)
    }
}
