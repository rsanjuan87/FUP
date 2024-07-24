package org.santech.fup

import android.content.ComponentName
import android.content.Intent
import android.graphics.drawable.Drawable
import android.os.Bundle
import android.provider.Settings
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.ImageBitmap
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.unit.dp
import androidx.core.graphics.drawable.toBitmap
import org.santech.fup.ui.theme.FUPTheme
import java.util.*
import java.util.concurrent.Delayed

class MainActivity : ComponentActivity(), UpdaterInterface {
    private var txt by mutableStateOf("")
    private var iconBitmap by mutableStateOf<ImageBitmap?>(null)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        NotificationService().setListener(this)
        setContent {
            FUPTheme {
                // A surface container using the 'background' color from the theme
                Surface(modifier = Modifier.fillMaxSize(), color = MaterialTheme.colorScheme.background) {
                    Column {
//                        Button(onClick = {
//                            val mNotificationManager: NotificationManager? =
//                                getSystemService(NOTIFICATION_SERVICE) as NotificationManager?
//                            val mBuilder: NotificationCompat.Builder =
//                                NotificationCompat.Builder(this@MainActivity, default_notification_channel_id)
//                            mBuilder.setContentTitle("My Notification")
//                            mBuilder.setContentText("Notification Listener Service Example")
//                            mBuilder.setTicker("Notification Listener Service Example")
//                            mBuilder.setSmallIcon(R.drawable.ic_launcher_foreground)
//                            mBuilder.setAutoCancel(true)
//                            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
//                                val importance: Int = NotificationManager.IMPORTANCE_HIGH
//                                val notificationChannel = NotificationChannel(
//                                    NOTIFICATION_CHANNEL_ID,
//                                    "NOTIFICATION_CHANNEL_NAME",
//                                    importance
//                                )
//                                mBuilder.setChannelId(NOTIFICATION_CHANNEL_ID)
//                                assert(mNotificationManager != null)
//                                mNotificationManager?.createNotificationChannel(notificationChannel)
//                            }
//                            assert(mNotificationManager != null)
//                            val id = System.currentTimeMillis().toInt()
//                            mNotificationManager?.notify(
//                                id,
//                                mBuilder.build()
//                            )
//                        }) {
//                            Text("Create Notification")
//                        }
//                        iconBitmap?.let {
//                            Image(bitmap = it, contentDescription = "App Icon")
//                        }
//                        Text(txt)
                        (iconBitmap ?: getIcon())?.let {
                            Image(
                                bitmap = it,
                                contentDescription = "App Icon",
                                modifier = Modifier.fillMaxSize().padding(50.dp),
                            )
                        }
                    }
                }
            }
        }
        // Verificar acceso al servicio de escucha de notificaciones
        if (!isNotificationServiceEnabled()) {
            openNotificationAccessSettings()
        } else {
            // El servicio de escucha de notificaciones está habilitado
        }

//        this.startService(Intent(this, NotificationService::class.java))
    }

    private fun getIcon(): ImageBitmap? {
        return getDrawable(R.mipmap.ic_launcher_foreground)?.toBitmap(1024, 1024)?.asImageBitmap()
    }

    private fun isNotificationServiceEnabled(): Boolean {
        val pkgName = packageName
        val flat = Settings.Secure.getString(contentResolver, "enabled_notification_listeners")
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

    override fun onResume() {
        super.onResume()
        Timer().schedule(object : TimerTask() {
            override fun run() {
                iconBitmap = null
            }
        }, 1000)
    }

    fun openNotificationAccessSettings() {
        // El servicio de escucha de notificaciones no está habilitado, redirigir para habilitar
        val intent = Intent(Settings.ACTION_NOTIFICATION_LISTENER_SETTINGS)
        startActivity(intent)
    }

    companion object {
        const val NOTIFICATION_CHANNEL_ID = "10001"
        private const val default_notification_channel_id = "default"
    }

    override fun updateApp(packageName: String?, drawableIcon: Drawable?) {
        txt += (" \n $packageName")
        val bitmap = drawableIcon?.toBitmap()
        val imageBitmap = bitmap?.asImageBitmap()
        iconBitmap = imageBitmap
    }
}



