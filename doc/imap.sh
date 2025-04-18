mail="info@swiss-os.com"
id="eaacf346cca81e150950b45c8925d3333daf0ce4"

token=`secret-tool lookup e-source-uid "${id}" | tr -d "\n"`
my_secret_password=`echo -en "\x00${mail}\x00${token}" | base64 | tr -d "\n"`

run=$(cat <<- END_OF_SESSION
a0 CAPABILITY
a1 AUTHENTICATE PLAIN ${my_secret_password}

a2 LIST "" "*"
a3 STATUS INBOX (MESSAGES)
a4 SELECT INBOX
a5 FETCH 1:* (FLAGS RFC822.HEADER) 
a6 FETCH 1 RFC822
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

(echo -n; sleep 1; echo "$run"; sleep 1) | sed -u "s/$/\r/g" | openssl s_client -connect mail.infomaniak.com:993 


