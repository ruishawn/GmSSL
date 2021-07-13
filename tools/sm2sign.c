/*
 * Copyright (c) 2021 - 2021 The GmSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the GmSSL Project.
 *    (http://gmssl.org/)"
 *
 * 4. The name "GmSSL Project" must not be used to endorse or promote
 *    products derived from this software without prior written
 *    permission. For written permission, please contact
 *    guanzhi1980@gmail.com.
 *
 * 5. Products derived from this software may not be called "GmSSL"
 *    nor may "GmSSL" appear in their names without prior written
 *    permission of the GmSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the GmSSL Project
 *    (http://gmssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE GmSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE GmSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <gmssl/sm2.h>
#include <gmssl/pem.h>
#include <gmssl/pkcs8.h>
#include <gmssl/error.h>

// echo data | sm2sign -id "Alice" -keyfile sm2.pem
// echo data | sm2verify -id "Alice" -keyfile sm2pub.pem -certfile a -cacertfile b




int main(int argc, char **argv)
{
	char *prog = basename(argv[0]);
	const char *id = SM2_DEFAULT_ID;
	const char *keyfile = NULL;
	FILE *keyfp = NULL;
	const char *pass = NULL;
	SM2_KEY key;
	SM2_SIGN_CTX sign_ctx;
	uint8_t buf[4096];
	ssize_t len;
	uint8_t sig[SM2_MAX_SIGNATURE_SIZE];
	size_t siglen;

	if (argc < 2) {
bad:
		fprintf(stderr, "%s : error options\n", prog);
help:
		fprintf(stderr, "usage: %s -key key.pem [-id str] < file\n", prog);
		return 1;
	}

	argc--;
	argv++;
	while (argc > 1) {
		if (!strcmp(*argv, "-help")) {
			goto help;
		} else if (!strcmp(*argv, "-id")) {
			if (--argc < 1) goto bad;
			id = *(++argv);
		} else if (!strcmp(*argv, "-key")) {
			if (--argc < 1) goto bad;
			keyfile = *(++argv);
		} else {
			goto help;
		}
		argc--;
		argv++;
	}

	if (!keyfile) {
		error_print();
		return -1;
	}
	if (!(keyfp = fopen(keyfile, "r"))) {
		error_print();
		return -1;
	}
	pass = getpass("Encryption Password : ");
	if (sm2_enced_private_key_info_from_pem(&key, pass, keyfp) != 1) {
		error_print("private key decryption failure");
		return -1;
	}

	sm2_sign_init(&sign_ctx, &key, id);
	while ((len = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
		sm2_sign_update(&sign_ctx, buf, len);
	}
	sm2_sign_finish(&sign_ctx, sig, &siglen);
	format_bytes(stdout, 0, 0, "", sig, siglen);
	return 0;

}
