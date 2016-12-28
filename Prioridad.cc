//Voip %pkt correctos +99% 150 ms retardo en una dirección y 30 ms jitter
//modelar silencios con codigo  (nivel 5)
//video streaming % pkt correctos +95%  ( nivel  4 )
//data con cuatro categorías 
//best effort 0 , bulk 1 , interactive 4, critical depende 
//Queuing guidelinesalso included not provisioning more than 33% of a link for realtime traffic and 25% for best effort class

//best effort  , HTTP WEB , NON CRITICAL 
//BULK database syscs, email(smtp), large ftp, network backups  (invoke TCP congestion) average message size 64kb or grater
//Transactional  SAP, ORACLE,.... CLIENT-SERVER model   ,   throw 1kb to 50mb
// interactive  telnet,AOL instant messenger  Average message < 100b ; max message < 1kb

//scavenger class , less than best effort , usually entertainment, like p2p media, gamming, or video (youtube)




// cisco /solutions /enterprise/ Wan and MAN/ QoS_SRND/QoS_SRND-Book/QoSIntro.html 
