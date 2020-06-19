#include "com_mcres_luckyfish_angustifolia_licenseclient_plugin_classloader_EncryptedClassLoader.h"

#include "main/main.h"

JNIEXPORT jbyteArray JNICALL Java_com_mcres_luckyfish_angustifolia_licenseclient_plugin_classloader_EncryptedClassLoader_analyzeClass(JNIEnv * jni, jclass class, jbyteArray data)
{
	return decryptClass(jni, data);
}
