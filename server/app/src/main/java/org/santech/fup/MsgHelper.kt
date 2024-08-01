package org.santech.fup

import android.app.Notification
import android.content.Context
import android.content.Intent
import android.content.pm.ApplicationInfo
import android.content.pm.PackageManager
import android.content.pm.ResolveInfo
import android.graphics.Bitmap
import android.graphics.Bitmap.CompressFormat
import android.graphics.drawable.Drawable
import android.service.notification.StatusBarNotification
import android.util.Log
import androidx.core.graphics.drawable.toBitmap
import org.santech.fup.NotificationService.Companion.updater
import java.io.File
import java.io.FileOutputStream
import kotlin.math.max


class MsgHelper {
    companion object {
        fun print(context: Context, tag: String, text: String) {
            Log.d(context.packageName, "$tag, $text")
        }

        fun printNotif(context: Context, tag: String, status: String, sbn: StatusBarNotification) {
            val draw = saveAppIcon(context, sbn.packageName)

            try {
                updater!!.updateApp("Post: " + sbn.packageName, draw)
            } catch (e: Exception) {
                e.printStackTrace()
            }
            var text = sbn.notification.extras.getCharSequence(Notification.EXTRA_TEXT).toString()
            if (text.isBlank())
                text = sbn.notification.tickerText.toString()
            var title = sbn.notification.extras.getString(Notification.EXTRA_TITLE)
            if (title.isNullOrBlank())
                title = getPkgName(context, sbn.packageName)
            print(
                context, tag,
                "{" +
                        "\"status\": \"$status\" \t " +
                        "\"NID\": \"${sbn.id}\" \t " +
                        "\"MSG\": \"${text}\" \t " +
                        "\"PKGID\": \"${sbn.packageName}\" \t " +
                        "\"PKGNAME\": \"${title}\" \t"
                        + "}"
            )
        }


        fun getPkgIcon(context: Context, pkgId: String): Drawable? {

            val pm = context.packageManager
            val ai: ApplicationInfo? = try {
                pm.getApplicationInfo(pkgId, PackageManager.GET_META_DATA)
            } catch (e: PackageManager.NameNotFoundException) {
                e.printStackTrace()
                return null
            }

            if (ai != null) {
                return pm.getApplicationIcon(ai)
            }
            return null
        }

        fun getPkgName(context: Context, pkgId: String): String? {

            val pm = context.packageManager
            val ai: ApplicationInfo? = try {
                pm.getApplicationInfo(pkgId, PackageManager.GET_META_DATA)
            } catch (e: PackageManager.NameNotFoundException) {
                e.printStackTrace()
                return null
            }

            if (ai != null) {
                return pm.getApplicationLabel(ai).toString()
            }
            return null
        }

        fun getLauncherApps(context: Context): List<ResolveInfo> {
            val pm = context.packageManager
            val intent = Intent(Intent.ACTION_MAIN, null).apply {
                addCategory(Intent.CATEGORY_LAUNCHER)
            }

            val launchers = pm.queryIntentActivities(intent, 0)
            return launchers
        }


        fun drawableToBitmap(pd: Drawable): Bitmap {
            return pd.toBitmap(max(512, pd.intrinsicWidth), max(512, pd.intrinsicHeight), Bitmap.Config.ARGB_8888)
        }

        fun saveAppIcon(context: Context, pkgId: String): Drawable {
            val draw: Drawable =
                (getPkgIcon(context, pkgId) ?: context.getDrawable(R.drawable.ic_launcher_foreground)) as Drawable
            saveIcon(context, pkgId, draw)
            return draw
        }

        fun saveIcon(context: Context, name: String, draw: Drawable) {
            val bm = drawableToBitmap(draw)
            val dir = Defs.KEY_FUP_ICONS_DIR(context)
            val file = File(dir, name)
            if (!dir.exists()) {
                dir.mkdirs()
            }
            if (file.length() <= 1L) {
                print(context, Defs.KEY_GET_PACKAGE_ICON, file.path)
                if (saveBitmapToFile(name, bm, CompressFormat.PNG, context)) {
                    print(context, Defs.KEY_SAVED_PACKAGE_ICON, file.name)
                } else {
                    Log.e("app", "Couldn't save icon.")
                }
            } else {
                Log.e("app", "Icon already saved.")
            }

        }

        /**
         * @param dir you can get from many places like Environment.getExternalStorageDirectory() or mContext.getFilesDir() depending on where you want to save the image.
         * @param fileName The file name.
         * @param bm The Bitmap you want to save.
         * @param format Bitmap.CompressFormat can be PNG,JPEG or WEBP.
         * @param quality quality goes from 1 to 100. (Percentage).
         * @return true if the Bitmap was saved successfully, false otherwise.
         */
        fun saveBitmapToFile(
            fileName: String, bm: Bitmap,
            format: CompressFormat,
            context: Context
        ): Boolean {
            try {
                val dir = Defs.KEY_FUP_ICONS_DIR(context)
                val imageFile = File(dir, fileName)
                if (!dir.exists()) {
                    dir.mkdirs()
                }
                var fos: FileOutputStream? = null
                if (imageFile.exists() && imageFile.length() <= 1L) {
                    imageFile.delete()
                }
                imageFile.createNewFile()
                fos = FileOutputStream(imageFile)
                bm.compress(format, 100, fos)
                fos.close()
                return true
            } catch (e: Exception) {
                e.printStackTrace()
            }
            return false
        }

    }
}
