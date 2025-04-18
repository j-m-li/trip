mail="swiss60373187111@gmail.com"

token=`secret-tool lookup e-source-uid "OAuth2::Google[${mail}]" | tr "," "\n" | grep access_token | tr '"' "\n" | grep '\.' | tr -d "\n"`
my_secret_password1=`echo -en "user=${mail}\x01auth=Bearer ${token}\x01\x01" | base64 | tr -d "\n"`

# echo "${token}"
# echo "${my_secret_password1}" | base64 -d

run=$(cat <<- END_OF_SESSION
a0 CAPABILITY
a1 AUTHENTICATE XOAUTH2 ${my_secret_password1}

a2 LIST "" "*"
a3 STATUS INBOX (MESSAGES)
a4 SELECT INBOX
a5 FETCH 1 (RFC822.HEADER RFC822.SIZE)
a5 FETCH 1:* (FLAGS)
a7 APPEND "INBOX" {26}
Subject: Yo

hello world
a8 CREATE my_folder
a9 RENAME my_folder myone
a10 COPY 2 myone
a11 SELECT myone
a12 FETCH 1 RFC822
a13 STORE 1 +FLAGS (\Deleted \Seen)
a13 CLOSE
a15 DELETE myone
a99 LOGOUT

END_OF_SESSION

)

(echo -n; sleep 2; echo "$run"; sleep 5) | sed -u "s/$/\r/g" | openssl s_client -connect imap.gmail.com:993 

exit
echo openssl s_client -crlf -connect imap.gmail.com:993
