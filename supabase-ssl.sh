# Retrieve the SSL certificate from supabase.co and convert it to a C header file
openssl s_client -showcerts -connect supabase.co:443 < /dev/null | openssl x509 -outform DER > root-ca.der
xxd -i root-ca.der > root-ca.h