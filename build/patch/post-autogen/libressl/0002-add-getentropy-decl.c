--- a/crypto/compat/arc4random.c	2016-12-06 01:16:11.000000000 +0100
+++ b/crypto/compat/arc4random.c	2016-12-06 01:16:25.000000000 +0100
@@ -82,6 +82,8 @@
 	chacha_ivsetup(&rsx->rs_chacha, buf + KEYSZ);
 }
 
+int getentropy(void *buf, size_t buflen); /* Forward decl. */
+
 static void
 _rs_stir(void)
 {
