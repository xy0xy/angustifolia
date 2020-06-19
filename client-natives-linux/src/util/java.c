#include "util.h"

JNIEnv * usedJni = NULL;

void setupJni(JNIEnv * jni)
{
	usedJni = jni;
}

char * fetchJarLocation()
{
	if (usedJni == NULL)
		return NULL;
	
	JNIEnv * env = usedJni;
	jclass jc = (*env)->FindClass(env, "com/mcres/luckyfish/angustifolia/licenseclient/plugin/PluginLicenseClient");
	jmethodID instanceMethod = (*env)->GetStaticMethodID(env, jc, "getInstance", "()Lcom/mcres/luckyfish/angustifolia/licenseclient/plugin/PluginLicenseClient;");
	jobject instance = (*env)->CallStaticObjectMethod(env, jc, instanceMethod);
	
	jmethodID method_getClass = (*env)->GetMethodID(env, jc, "getClass", "()Ljava/lang/Class;");
	jobject   obj_Class       = (*env)->CallObjectMethod(env, instance, method_getClass);
	
	jclass    cls_Class                  = (*env)->GetObjectClass(env, obj_Class);
	jmethodID method_getProtectionDomain = (*env)->GetMethodID(env, cls_Class, "getProtectionDomain", "()Ljava/security/ProtectionDomain;");
	jobject   obj_ProtectionDomain       = (*env)->CallObjectMethod(env, obj_Class, method_getProtectionDomain);
	
	jclass    cls_ProtectionDomain = (*env)->GetObjectClass(env, obj_ProtectionDomain);
	jmethodID method_getCodeSource = (*env)->GetMethodID(env, cls_ProtectionDomain, "getCodeSource", "()Ljava/security/CodeSource;");
	jobject   obj_CodeSource       = (*env)->CallObjectMethod(env, obj_ProtectionDomain, method_getCodeSource);
	
	jclass    cls_CodeSource     = (*env)->GetObjectClass(env, obj_CodeSource);
	jmethodID method_getLocation = (*env)->GetMethodID(env, cls_CodeSource, "getLocation", "()Ljava/net/URL;");
	jobject   obj_URL            = (*env)->CallObjectMethod(env, obj_CodeSource, method_getLocation);
	
	jclass    cls_URL      = (*env)->GetObjectClass(env, obj_URL);
	jmethodID method_toURI = (*env)->GetMethodID(env, cls_URL, "toURI", "()Ljava/net/URI;");
	jobject   obj_URI      = (*env)->CallObjectMethod(env, obj_URL, method_toURI);
	
	jclass    cls_File         = (*env)->FindClass (env, "java/io/File");
	jmethodID method_File_init = (*env)->GetMethodID(env, cls_File, "<init>", "(Ljava/net/URI;)V");
	jobject   obj_File         = (*env)->NewObject(env, cls_File, method_File_init, obj_URI);
	
	jmethodID method_getPath = (*env)->GetMethodID(env, cls_File, "getPath", "()Ljava/lang/String;");
	jobject   obj_String     = (*env)->CallObjectMethod(env, obj_File, method_getPath);
	
	const char * jarLoc = (*env)->GetStringUTFChars(env, obj_String, 0);
	size_t length = (*env)->GetStringUTFLength(env, obj_String);
	char * result = malloc(sizeof(char) * length);
	memset(result, 0, sizeof(char) * length);
	memcpy(result, jarLoc, sizeof(char) * length);
	(*env)->ReleaseStringUTFChars(env, obj_String, jarLoc);
	return result;
}

char * getDataFolder()
{
	if (usedJni == NULL)
		return NULL;
	JNIEnv * env = usedJni;
	jclass jc = (*env)->FindClass(env, "com/mcres/luckyfish/angustifolia/licenseclient/plugin/PluginLicenseClient");
	jthrowable jt = (*env)->ExceptionOccurred(env);
	if (jt)
	{
		(*env)->Throw(env, jt);
		return NULL;
	}
	jmethodID instanceMethod = (*env)->GetStaticMethodID(env, jc, "getInstance", "()Lcom/mcres/luckyfish/angustifolia/licenseclient/plugin/PluginLicenseClient;");
	jobject instance = (*env)->CallStaticObjectMethod(env, jc, instanceMethod);
	
	jmethodID method_getDataFolder = (*env)->GetMethodID(env, jc, "getDataFolder", "()Ljava/io/File;");
	jobject obj_dataFolder = (*env)->CallObjectMethod(env, instance, method_getDataFolder);
	
	jclass cls_File = (*env)->GetObjectClass(env, obj_dataFolder);
	jmethodID method_getAbsolutePath = (*env)->GetMethodID(env, cls_File, "getAbsolutePath", "()Ljava/lang/String;");
	jobject obj_String = (*env)->CallObjectMethod(env, obj_dataFolder, method_getAbsolutePath);
	
	const char * dataFolder = (*env)->GetStringUTFChars(env, obj_String, 0);
	size_t length = (*env)->GetStringUTFLength(env, obj_String) + 1;
	char * result = malloc(sizeof(char) * length);
	memset(result, 0, sizeof(char) * length);
	memcpy(result, dataFolder, length * sizeof(char));
	(*env)->ReleaseStringUTFChars(env, obj_String, dataFolder);
	return result;
}

bool fetchResourceInfo(unsigned int * userId, unsigned int * resourceId, char ** order, size_t * orderLen)
{
	if (usedJni == NULL)
		return false;
	
	JNIEnv *env = usedJni;
	jclass jc = (*env)->FindClass(env, "com/mcres/luckyfish/angustifolia/licenseclient/plugin/PluginLicenseClient");
	
	jfieldID field_userId = (*env)->GetStaticFieldID(env, jc, "userId", "I");
	jint userIdVal = (*env)->GetStaticIntField(env, jc, field_userId);
	
	jfieldID field_resourceId = (*env)->GetStaticFieldID(env, jc, "resourceId", "I");
	jint resourceIdVal = (*env)->GetStaticIntField(env, jc, field_resourceId);
	
	if (order != NULL)
	{
		jfieldID field_order = (*env)->GetStaticFieldID(env, jc, "order", "Ljava/lang/String;");
		jstring orderVal = (*env)->GetStaticObjectField(env, jc, field_order);
		
		*orderLen = sizeof(char) * (*env)->GetStringUTFLength(env, orderVal);
		
		const char *resultOrder = (*env)->GetStringUTFChars(env, orderVal, 0);
		*order = malloc(*orderLen);
		memcpy(*order, resultOrder, *orderLen);
		(*env)->ReleaseStringChars(env, orderVal, resultOrder);
	}
	
	*userId = (unsigned int) userIdVal;
	*resourceId = (unsigned int) resourceIdVal;
	
	return true;
}

void callJavaStringMethod(char * class, char * name, char * str)
{
	if (usedJni == NULL)
		return;
	JNIEnv  * env = usedJni;
	jclass jc = (*env)->FindClass(env, class);
	jmethodID method = (*env)->GetMethodID(env, jc, name, "(Ljava/lang/String;)V");
	jstring jstr= (*env)->NewStringUTF(env, str);
	(*env)->CallVoidMethod(env, NULL, method, jstr);
}

