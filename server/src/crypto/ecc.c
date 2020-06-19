#include "ecc.h"
#include <openssl/sha.h>
#include <string.h>
#include <math.h>

#include <stdio.h>
#include <errno.h>
#include <assert.h>

void * crackBigNum(const BIGNUM * bn, size_t * length);
BIGNUM * readBigNum(void * data, size_t length);

WrappedKey generateKey()
{
	EC_KEY * key;
	EC_POINT * qPoint;
	
	BIGNUM * p = BN_new();
	BIGNUM * a = BN_new();
	BIGNUM * b = BN_new();
	
	BIGNUM * zero = BN_new();
	BN_zero(zero);
	BIGNUM * one = BN_new();
	BN_one(one);
	BIGNUM * two = BN_new();
	BN_add(two, one, one);
	BIGNUM * three = BN_new();
	BN_add(three, two, one);
	BIGNUM * four = BN_new();
	BN_add(four, two, two);
	
	BN_CTX * ctx = BN_CTX_new();
	
	EC_METHOD * method = (EC_METHOD *)EC_GFp_simple_method();
	EC_GROUP * group = EC_GROUP_new(method);
	
	BN_hex2bn(&p, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F");
	BN_dec2bn(&a, "0");
	BN_dec2bn(&b, "7");
	
	EC_GROUP_set_curve(group, p, a, b, ctx);
	
	EC_POINT * generator = EC_POINT_new(group);
	BIGNUM * generator_x = BN_new();
	BIGNUM * generator_y = BN_new();
	BN_hex2bn(&generator_x, "79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798");
	BN_hex2bn(&generator_y, "483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8");
	EC_POINT_set_affine_coordinates(group, generator, generator_x, generator_y, ctx);
	
	BIGNUM * order = BN_new();
	BN_hex2bn(&order, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141");
	BIGNUM * cofactor = BN_new();
	BN_copy(cofactor, one);
	
	EC_GROUP_set_generator(group, generator, order, cofactor);
	
	EC_GROUP_set_curve_name(group, 0x114514);
	
	key = EC_KEY_new();
	EC_KEY_set_group(key, group);
	
	EC_KEY_generate_key(key);
	
	qPoint = EC_POINT_new(group);
	EC_POINT_mul(group, qPoint, NULL, EC_KEY_get0_public_key(key), EC_KEY_get0_private_key(key), ctx);
	
	WrappedKey ecKey;
	ecKey.key = key;
	ecKey.pointQ = qPoint;
	ecKey.group.curveGroup = group;
	ecKey.group.bigNumberContext = ctx;
	
	BN_free(zero);
	BN_free(one);
	BN_free(two);
	BN_free(three);
	BN_free(four);
	
	return ecKey;
}

void * encrypt(void * data, size_t length, size_t * encrypted_length, WrappedKey key)
{
	EC_KEY * internalKey = key.key;
	// public key to encrypt, private key to decrypt
	EC_POINT * publicKey = EC_KEY_get0_public_key(internalKey);
	
	EC_POINT * kP = EC_POINT_new(key.group.curveGroup);
	EC_POINT * kQ = EC_POINT_new(key.group.curveGroup);
	
	BIGNUM * k = BN_new();
	BIGNUM * n = BN_new();
	BIGNUM * p = BN_new();
	BIGNUM * a = BN_new();
	BIGNUM * b = BN_new();
	
	EC_GROUP_get_curve(key.group.curveGroup, p, a, b, key.group.bigNumberContext);
	EC_GROUP_get_order(key.group.curveGroup, n, key.group.bigNumberContext);
	
	BIGNUM * x_kq = BN_new();
	BIGNUM * y_kq = BN_new();
	
	BIGNUM * x_q = BN_new();
	BIGNUM * y_q = BN_new();
	
	EC_POINT_get_affine_coordinates(key.group.curveGroup, key.pointQ, x_q, y_q, key.group.bigNumberContext);
	assert((!BN_is_zero(x_q)) || (!BN_is_zero(y_q)));
	
	BN_free(x_q);
	BN_free(y_q);
	
	BIGNUM * one = BN_new();
	BN_one(one);
	
	BIGNUM * n_1 = BN_new();
	BN_sub(n_1, n, one);
	
	do
	{
		BN_rand_range(k, n_1);
		BN_add(k, k, one);
		
		EC_POINT_mul(key.group.curveGroup, kP, NULL, publicKey, k, key.group.bigNumberContext);
		EC_POINT_mul(key.group.curveGroup, kQ, NULL, key.pointQ, k, key.group.bigNumberContext);
		
		EC_POINT_get_affine_coordinates(key.group.curveGroup, kQ, x_kq, y_kq, key.group.bigNumberContext);
		if (!BN_is_zero(x_kq))
			break;
	} while (1);
	
	CipherData cipherData;
	cipherData.pPoint = kP;
	cipherData.dataLen = length;
	cipherData.data = malloc(sizeof(BIGNUM *) * cipherData.dataLen);
	memset(cipherData.data, 0, sizeof(BIGNUM *) * cipherData.dataLen);
#ifdef TEST_ECC
	printf("##ENCRYPT START##\n");
#endif // TEST_ECC
	for (size_t i = 0; i < length; i ++)
	{
		char encrypt_target = ((char *)data)[i];
		BIGNUM * encrypted = BN_new();
		
		BN_zero(encrypted);
		BN_add_word(encrypted, (unsigned)encrypt_target);
		
		BN_mul(encrypted, encrypted, x_kq, key.group.bigNumberContext);
		BN_mod(encrypted, encrypted, p, key.group.bigNumberContext);

#ifdef TEST_ECC
		printf("'%c' -> %s\n", encrypt_target, BN_bn2hex(encrypted));
#endif // TEST_ECC
		cipherData.data[i] = encrypted;
	}

#ifdef TEST_ECC
	printf("##ENCRYPT END##\n");
#endif // TEST_ECC
	
	EC_POINT_free(kQ);
	
	BN_free(p);
	BN_free(a);
	BN_free(b);
	BN_free(k);
	
	BN_free(x_kq);
	BN_free(y_kq);
	BN_free(one);
	BN_free(n);
	BN_free(n_1);
	
	// extract BN data
	BIGNUM * pPointBN = BN_new();
	EC_POINT_point2bn(key.group.curveGroup, cipherData.pPoint, POINT_CONVERSION_UNCOMPRESSED, pPointBN, key.group.bigNumberContext);
	
	size_t rawPPointLen;
	void * rawPPoint = crackBigNum(pPointBN, &rawPPointLen);
	
	size_t * dataLens = malloc(cipherData.dataLen * sizeof(size_t));
	void ** dataList = malloc(sizeof(void *) * cipherData.dataLen);
	
	for (size_t i = 0; i < cipherData.dataLen; i ++)
	{
		dataList[i] = crackBigNum(cipherData.data[i], &dataLens[i]);
	}
	
	// connect everything together
	size_t finalSize = sizeof(size_t) + rawPPointLen + sizeof(rawPPointLen);
	for (size_t i = 0; i < cipherData.dataLen; i ++)
	{
		finalSize += dataLens[i] + sizeof(size_t);
	}
	
	void * finalData = malloc(finalSize);
	unsigned long mark = (unsigned long) finalData;
	memcpy((void *) mark, &cipherData.dataLen, sizeof(size_t));
	mark += sizeof(size_t);
	
	memcpy((void *) mark, &rawPPointLen, sizeof(size_t));
	mark += sizeof(size_t);
	memcpy((void *) mark, rawPPoint, rawPPointLen);
	mark += rawPPointLen;
	
	for (size_t i = 0; i < cipherData.dataLen; i ++)
	{
		memcpy((void *) mark, &dataLens[i], sizeof(size_t));
		mark += sizeof(size_t);
		memcpy((void *) mark, dataList[i], dataLens[i]);
		mark += dataLens[i];
		free(dataList[i]);
	}
	free(rawPPoint);
	free(dataList);
	free(dataLens);
	
	if (encrypted_length)
		*encrypted_length = finalSize;
	
	return finalData;
}

void * decrypt(void * rawData, size_t * decrypted_length, WrappedKey key)
{
	CipherData data;
	
	// initialize CipherData
	unsigned long mark = rawData;
	memcpy(&data.dataLen, (void *) mark, sizeof(size_t));
	mark += sizeof(size_t);
	
	size_t pPointDataLen;
	memcpy(&pPointDataLen, (void *) mark, sizeof(size_t));
	mark += sizeof(size_t);
	
	void * rawPPointData = malloc(pPointDataLen);
	memcpy(rawPPointData, (void *) mark, pPointDataLen);
	mark += pPointDataLen;
	
	BIGNUM * pPointBn = readBigNum(rawPPointData, pPointDataLen);
	data.pPoint = EC_POINT_new(key.group.curveGroup);
	EC_POINT_bn2point(key.group.curveGroup, pPointBn, data.pPoint, key.group.bigNumberContext);
	free(rawPPointData);
	
	// restore everything we have.
	data.data = malloc(sizeof(BIGNUM *) * data.dataLen);
	for (size_t i = 0; i < data.dataLen; i ++)
	{
		size_t length;
		memcpy(&length, (void *) mark, sizeof(size_t));
		mark += sizeof(size_t);
		
		void * rawBNData = malloc(length);
		memcpy(rawBNData, (void *) mark, length);
		BIGNUM * n = readBigNum(rawBNData, length);
		data.data[i] = n;
		mark += length;
		free(rawBNData);
	}
	
	// start decrypt
	
	BIGNUM * secret_key = BN_secure_new();
	void * result = malloc(data.dataLen);
	void * edit = result;
	BN_copy(secret_key, EC_KEY_get0_private_key(key.key));
	
	EC_POINT * pt1 = EC_POINT_new(key.group.curveGroup);
	EC_POINT * pt2 = EC_POINT_new(key.group.curveGroup);
	
	EC_POINT_copy(pt1, data.pPoint);
	EC_POINT_mul(key.group.curveGroup, pt2, NULL, pt1, secret_key, key.group.bigNumberContext);
	
	BIGNUM * pt2_x = BN_new();
	BIGNUM * pt2_y = BN_new();
	
	EC_POINT_get_affine_coordinates(key.group.curveGroup, pt2, pt2_x, pt2_y, key.group.bigNumberContext);
	
	BIGNUM * p = BN_new();
	BIGNUM * a = BN_new();
	BIGNUM * b = BN_new();
	
	BIGNUM * zero = BN_new();
	BN_zero(zero);
	
	EC_GROUP_get_curve(key.group.curveGroup, p, a, b, key.group.bigNumberContext);
#ifdef TEST_ECC
	printf("##DECRYPT START##\n");
#endif // TEST_ECC
	
	for (size_t i = 0; i < data.dataLen; i ++)
	{
		BIGNUM * m = BN_new();
		
		BN_mod_inverse(m, pt2_x, p, key.group.bigNumberContext);
		BN_mul(m, m, data.data[i], key.group.bigNumberContext);
		BN_mod(m, m, p, key.group.bigNumberContext);
		
		if (BN_cmp(m, zero) < 0)
			BN_add(m, m, p);
		
		char * temp = edit ++;
		*temp = (char) atoi(BN_bn2dec(m));

#ifdef TEST_ECC
		printf("%s -> '%c'\n", BN_bn2hex(m), *temp);
#endif // TEST_ECC
		BN_free(m);
	}
#ifdef TEST_ECC
	printf("##DECRYPT_END##\n");
#endif
	
	BN_free(pt2_x);
	BN_free(pt2_y);
	BN_free(p);
	BN_free(a);
	BN_free(b);
	BN_free(zero);
	
	EC_POINT_free(pt1);
	EC_POINT_free(pt2);
	BN_free(secret_key);
	
	if (decrypted_length)
		*decrypted_length = data.dataLen;
	
	return result;
}

void saveKey(WrappedKey key, char * filename)
{
	FILE * f = fopen(filename, "w");
	if (!f)
	{
		printf("We cannot open the file called %s, error code: %d\n", filename, errno);
		return;
	}
	
	char * pointHex = EC_POINT_point2hex(key.group.curveGroup, EC_KEY_get0_public_key(key.key), POINT_CONVERSION_UNCOMPRESSED, key.group.bigNumberContext);
	fprintf(f, "%ld\n", strlen(pointHex));
	fputs(pointHex, f);
	OPENSSL_free(pointHex);
	fputs("\n", f);
	
	char * privateKeyHex = BN_bn2hex(EC_KEY_get0_private_key(key.key));
	fprintf(f, "%ld\n", strlen(privateKeyHex));
	fputs(privateKeyHex, f);
	OPENSSL_free(privateKeyHex);
	fputs("\n", f);
	
	fclose(f);
}

WrappedKey readKey(WrappedGroup group, char * filename)
{
	WrappedKey result;
	FILE * f = fopen(filename, "r");
	if (!f)
	{
		result.key = NULL;
		return result;
	}
	
	size_t pointHexLen;
	fscanf(f, "%ld", &pointHexLen);
	char * pointHex = malloc(sizeof(char) * pointHexLen);
	
	int sizeLen = snprintf(NULL, 0, "%lu", pointHexLen);
	char * format = malloc(sizeof(char) * (sizeLen + 3));
	memset(format, 0, sizeof(char) * (sizeLen + 3));
	snprintf(format, sizeLen + 3, "%%%lus", pointHexLen);
	
	fscanf(f, format, pointHex);
	free(format);
	
	size_t privateKeyHexLen;
	fscanf(f, "%ld", &privateKeyHexLen);
	sizeLen = snprintf(NULL, 0, "%lu", pointHexLen);
	format = malloc(sizeof(char) * (sizeLen + 3));
	memset(format, 0, sizeof(char) * (sizeLen + 3));
	snprintf(format, sizeLen + 3, "%%%lus", pointHexLen);
	
	char * privateKeyHex = malloc(sizeof(char) * privateKeyHexLen);
	fscanf(f, format, privateKeyHex);
	free(format);
	
	fclose(f);
	
	EC_KEY * key = EC_KEY_new();
	EC_KEY_set_group(key, group.curveGroup);
	
	EC_POINT * publicKey = EC_POINT_new(group.curveGroup);
	EC_POINT_hex2point(group.curveGroup, pointHex, publicKey, group.bigNumberContext);
	
	BIGNUM * privateKey = BN_new();
	BN_hex2bn(&privateKey, privateKeyHex);
	
	EC_KEY_set_public_key(key, publicKey);
	EC_KEY_set_private_key(key, privateKey);
	
	EC_POINT * pq = EC_POINT_new(group.curveGroup);
	EC_POINT_mul(group.curveGroup, pq, NULL, EC_KEY_get0_public_key(key), EC_KEY_get0_private_key(key), group.bigNumberContext);
	
	free(pointHex);
	free(privateKeyHex);
	
	result.key = key;
	result.pointQ = pq;
	result.group = group;
	
	return result;
}

bool isKeyValid(WrappedKey key)
{
	return key.key != NULL && key.group.curveGroup != NULL && key.pointQ != NULL && key.group.bigNumberContext != NULL && EC_KEY_check_key(key.key);
}

void * crackBigNum(const BIGNUM * bn, size_t * length)
{
	void * dataList = *(void **)bn;
	unsigned long address = (unsigned long) bn;
	address += sizeof(BN_ULONG *);
	
	int top = *(int *) address;
	address += sizeof(int);
	int data_len = *(int *) address;
	size_t len_result = sizeof(int) * 3 + sizeof(BN_ULONG) * data_len;
	
	if (length)
		*length = len_result;
	
	void * data = malloc(len_result);
	unsigned long mark = (unsigned long) data;
	bool isNeg = BN_is_negative(bn) ? true : false;
	memcpy((void *) mark, &isNeg, sizeof(int));
	
	mark += sizeof(int);
	memcpy((void *) mark, &top, sizeof(int));
	
	mark += sizeof(int);
	memcpy((void *) mark, &data_len, sizeof(int));
	
	mark += sizeof(int);
	memcpy((void *) mark, dataList, sizeof(BN_ULONG) * data_len);
	
	return data;
}

void * getPublicKey(WrappedKey key, size_t * length)
{
	const EC_POINT *publicKey = EC_KEY_get0_public_key(key.key);
	if (!publicKey)
		return NULL;
	
	BIGNUM * pointBn = BN_new();
	EC_POINT_point2bn(key.group.curveGroup, publicKey, POINT_CONVERSION_UNCOMPRESSED, pointBn, key.group.bigNumberContext);
	size_t pkLen;
	void * pkData = crackBigNum(pointBn, &pkLen);
	BIGNUM * pointQBn = BN_new();
	EC_POINT_point2bn(key.group.curveGroup, key.pointQ, POINT_CONVERSION_UNCOMPRESSED, pointQBn, key.group.bigNumberContext);
	size_t pqLen;
	void * pqData = crackBigNum(pointQBn, &pqLen);
	
	void * resultData = malloc(sizeof(size_t) * 2 + pkLen + pqLen);
	unsigned long address = (unsigned long) resultData;
	memcpy((void *) address, &pkLen, sizeof(size_t));
	address += sizeof(size_t);
	memcpy((void *) address, pkData, pkLen);
	address += pkLen;
	memcpy((void *) address, &pqLen, sizeof(size_t));
	address += sizeof(size_t);
	memcpy((void *) address, pqData, pqLen);
	
	free(pkData);
	free(pqData);
	
	*length = pkLen + pqLen + 2 * sizeof(size_t);
	
	return resultData;
}

BIGNUM * encodeBigNum(void * data, size_t length)
{
	BIGNUM * bn = BN_new();
	unsigned long mark = (unsigned long) bn;
	// maybe a little bigger, but it should be fine.
	unsigned int size = sizeof(BN_ULONG) * (int)ceil((0.1 + length + sizeof(size_t)) / sizeof(BN_ULONG));
	
	// fills the data
	*((void **) bn) = malloc(size);
	memset(*((void **)bn), 0, size);
	unsigned long dataMark = *((void **) bn);
	memcpy((void *) dataMark, &length, sizeof(size_t));
	dataMark += sizeof(size_t);
	memcpy((void *) dataMark, data, length);
	mark += sizeof(BN_ULONG *);
	
	int top = 1;
	memcpy((void *) mark, &top, sizeof(int));
	mark += sizeof(int);
	
	size /= sizeof(BN_ULONG);
	
	memcpy((void *) mark, &size, sizeof(int));
	
	return bn;
}

void decodeBigNum(BIGNUM * bn, void ** data, size_t * length)
{
	unsigned long mark = bn;
	unsigned long dataMark = *(unsigned long *)bn;
	
	// skip the top
	mark += sizeof(int);
	
	unsigned int dataLen = *(unsigned int *) mark;
	
	memcpy(length, (size_t *)dataMark, sizeof(size_t));
	dataMark += sizeof(size_t);
	*data = malloc(*length);
	memcpy(*data, (void *) dataMark, *length);
}

BIGNUM * readBigNum(void * data, size_t length)
{
	if (length < sizeof(int) + sizeof(int))
		return NULL;
	
	// is_neg
	int negInt = 0;
	memcpy(&negInt, data, sizeof(int));
	
	unsigned long mark = (unsigned long) data;
	mark += sizeof(int);
	
	int top = 0;
	memcpy(&top, (void *)mark, sizeof(int));
	mark += sizeof(int);
	int len = 0;
	memcpy(&len, (void *)mark, sizeof(int));
	if (len == 0 && length - sizeof(int) - sizeof(int) > 0)
		return NULL; // err > <
	mark += sizeof(int);
	
	BIGNUM * val = BN_new();
	
	BN_set_negative(val, negInt);
	*((void **)val) = malloc(sizeof(BN_ULONG) * len);
	
	memcpy(*((void **)val), (const void *) mark, sizeof(BN_ULONG) * len);
	
	unsigned long address = (unsigned long) val;
	address += sizeof(BN_ULONG *);
	memcpy((void *)address, &top, sizeof(int));
	address += sizeof(int);
	memcpy((void *)address, &len, sizeof(int));

#ifdef TEST_ECC
	printf("Data: %s\n", BN_bn2dec(val));
#endif // TEST_ECC
	
	return val;
}

WrappedKey publicKeyToWrappedKey(void *data, size_t length, WrappedGroup group)
{
	WrappedKey wk;
	if (!data)
	{
		wk.key = NULL;
		return wk;
	}
	
	unsigned long address = (unsigned long) data;
	size_t pkLen;
	memcpy(&pkLen, (void *) address, sizeof(size_t));
	void * pkData = malloc(pkLen);
	address += sizeof(size_t);
	memcpy(pkData, (void *) address, pkLen);
	
	BIGNUM * publicKeyBn = readBigNum(pkData, pkLen);
	
	free(pkData);
	address += pkLen;
	
	size_t pqLen;
	memcpy(&pqLen, (void *) address, sizeof(size_t));
	void * pqData = malloc(pqLen);
	address += sizeof(size_t);
	memcpy(pqData, (void *) address, pqLen);
	
	BIGNUM * pointQBn = readBigNum(pqData, pqLen);
	free(pqData);

#ifdef TEST_ECC
	printf("Pk bn: %s\n", BN_bn2hex(publicKeyBn));
#endif // TEST_ECC
	EC_POINT * publicKeyPoint = EC_POINT_new(group.curveGroup);
	
	EC_POINT_bn2point(group.curveGroup, publicKeyBn, publicKeyPoint, group.bigNumberContext);
	EC_KEY * pubKey = EC_KEY_new();
	EC_KEY_set_group(pubKey, group.curveGroup);
	EC_KEY_set_public_key(pubKey, publicKeyPoint);
	
	EC_POINT * pointQ = EC_POINT_new(group.curveGroup);
	
	EC_POINT_bn2point(group.curveGroup, pointQBn, pointQ, group.bigNumberContext);
	
	BIGNUM * x_q = BN_new();
	BIGNUM * y_q = BN_new();
	
	EC_POINT_get_affine_coordinates(group.curveGroup, pointQ, x_q, y_q, group.bigNumberContext);
	assert((!BN_is_zero(x_q)) || (!BN_is_zero(y_q)));
	
	BN_free(x_q);
	BN_free(y_q);
	
	wk.key = pubKey;
	wk.pointQ = pointQ;
	wk.group = group;
	
	return wk;
}

void * sign(void * data, size_t data_length, size_t * signature_length, WrappedKey key)
{
	BIGNUM * k = BN_new();
	
	BIGNUM * wrappedData = encodeBigNum(data, data_length);
	
	BIGNUM * r = BN_new();
	BIGNUM * s = BN_new();
	
	BN_zero(r);
	BN_zero(s);
	
	BIGNUM * zero = BN_new();
	BIGNUM * one = BN_new();
	
	BN_zero(zero);
	BN_one(one);
	
	BIGNUM * n = BN_new();
	EC_GROUP_get_order(key.group.curveGroup, n, key.group.bigNumberContext);
	BIGNUM * n_1 = BN_new();
	BN_sub(n_1, n, one);
	
	do
	{
		BN_rand_range(k, n_1);
		BIGNUM * temp2 = BN_new();
		BN_add(temp2, k, one);
		BN_free(k);
		k = temp2;
		
		EC_POINT * temp = EC_POINT_new(key.group.curveGroup);
		EC_POINT_mul(key.group.curveGroup, temp, k, NULL, NULL, key.group.bigNumberContext);
		
		BIGNUM * x = BN_new();
		BIGNUM * y = BN_new();
		
		EC_POINT_get_affine_coordinates(key.group.curveGroup, temp, x, y, key.group.bigNumberContext);
		BN_mod(r, x, n, key.group.bigNumberContext);
		BN_free(y);
		BN_free(x);
		
		if (BN_cmp(r, zero) < 0)
		{
			temp2 = BN_new();
			BN_add(temp2, r, n);
			BN_free(r);
			r = temp2;
		}
		
		temp2 = BN_new();
		BIGNUM * temp3 = BN_new();
		BN_mul(temp3, r, EC_KEY_get0_private_key(key.key), key.group.bigNumberContext);
		BN_add(temp2, temp3, wrappedData);
		BIGNUM * temp4 = BN_new();
		BN_mod_inverse(temp4, k, n, key.group.bigNumberContext);
		BIGNUM * temp5 = BN_new();
		BN_mul(temp5, temp2, temp4, key.group.bigNumberContext);
		BN_mod(s, temp5, n, key.group.bigNumberContext);
		
		BN_free(temp2);
		BN_free(temp3);
		BN_free(temp4);
		BN_free(temp5);
		
		if (BN_cmp(s, zero) < 0)
		{
			temp2 = BN_new();
			BN_add(temp2, s, n);
			BN_free(s);
			s = temp2;
		}
	} while ((!BN_cmp(r, zero)) || (!BN_cmp(s, zero)));
	
	// Ok move signature together.
	size_t rLen;
	void * rawR = crackBigNum(r, &rLen);
	size_t sLen;
	void * rawS = crackBigNum(s, &sLen);
	
	void * signature = malloc(rLen + sLen + 2 * sizeof(size_t));
	unsigned long address = (unsigned long)signature;
	
	memcpy((void *) address, &rLen, sizeof(size_t));
	address += sizeof(size_t);
	memcpy((void *) address, rawR, rLen);
	address += rLen;
	memcpy((void *) address, &sLen, sizeof(size_t));
	address += sizeof(size_t);
	memcpy((void *) address, rawS, sLen);
	
	free(rawR);
	free(rawS);
	
	*signature_length = rLen + sLen + 2 * sizeof(size_t);
	
	return signature;
}

bool verify(void * signature, void * data, size_t dataLength, WrappedKey key)
{
	BIGNUM * u = BN_secure_new();
	BIGNUM * v = BN_secure_new();
	BIGNUM * r = BN_new();
	
	BIGNUM * n = BN_new();
	EC_GROUP_get_order(key.group.curveGroup, n, key.group.bigNumberContext);
	
	unsigned long address = (unsigned long) signature;
	size_t signRLen;
	memcpy(&signRLen, (void *) address, sizeof(size_t));
	address += sizeof(size_t);
	
	BIGNUM * signR = readBigNum((void *) address, signRLen);
	address += signRLen;
	
	size_t signSLen;
	memcpy(&signSLen, (void *) address, sizeof(size_t));
	address += sizeof(size_t);
	
	BIGNUM * signS = readBigNum((void *) address, signSLen);
	
	BIGNUM * wrappedData = encodeBigNum(data, dataLength);
	
	BIGNUM * temp1 = BN_new();
	BIGNUM * temp2 = BN_new();
	
	BN_mod_inverse(temp1, signS, n, key.group.bigNumberContext);
	BN_mul(temp2, temp1, wrappedData, key.group.bigNumberContext);
	BN_mod(u, temp2, n, key.group.bigNumberContext);
	
	BN_mul(temp2, temp1, signR, key.group.bigNumberContext);
	BN_mod(v, temp2, n, key.group.bigNumberContext);
	
	EC_POINT * temp3 = EC_POINT_new(key.group.curveGroup);
	
	EC_POINT * pk = EC_KEY_get0_public_key(key.key);
	
	EC_POINT_mul(key.group.curveGroup, temp3, u, pk, v, key.group.bigNumberContext);
	
	EC_POINT_get_affine_coordinates(key.group.curveGroup, temp3, temp1, temp2, key.group.bigNumberContext);
	BN_mod(r, temp1, n, key.group.bigNumberContext);
	
	bool valid = !BN_cmp(r, signR);
	
	BN_free(r);
	BN_free(u);
	BN_free(v);
	BN_free(n);
	
	EC_POINT_free(temp3);
//	EC_POINT_free(pk);
	
	BN_free(temp1);
	BN_free(temp2);
	
	BN_free(signR);
	BN_free(signS);
	
	return valid;
}
