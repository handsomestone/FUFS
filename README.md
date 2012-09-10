===================================================================
          _____            _____     _____ 
	     |  ___| | | | |  |  ___|   / ____|
         | |___  | | | |  | |___   | |__ _
         |  ___| | | | |  |  ___|   \____  \
         | |     | |_| |  | |        ____| |
         |_|      \___/   |_|       |____ / 
===================================================================

fufs is a filesystem for accessing sina vdisk based on FUSE.
(You can use it in commercial software, but please let me know.)

How to build fufs from git source code?
===================================================================
1 prepare tools before configure
  if you use ubuntu operating system.you can you apt-get install
  to fetch the basic tools for fufs.
  - libcurl will be used for fufs to send http request or receive http answers
  - json-c will be used for parse response of sina app engine
  - glib will be used for hash table
  
  add your account and password in fufs.c
  example : your sina disk account is : example@gmail.com
            your sina disk password is: 111111
  you should change the main function in fufs.c
  line 523:	 fufs_api_get_token("input your account","input your password","local");
  add your account and password:
  line 523: fufs_api_get_token("example@gmail.com","111111","local");
  
  
2 configure
===================================================================
  cd fufs

  ./autogen.sh
  
  ./configure
   
3 build
===================================================================
  make

4 install (need root)
===================================================================
  make install  
  
5 run:
===================================================================
  mkdir /tmp/fufs
  
  cd src/
  
  ./fufs /tmp/fufs

 
  after all these steps you could have successfully installed fufs on your linux desktop
  if your account and password is right . you could find your folder and file which are stored in the sina server engine.
  you can read or write the file in the /tmp/fufs as you like.
  
 note:
  after you have run your fufs.you should start a small tool which is in the subfolder of fufs/src,which is called token_keep_thread
  you can change the source code int token_keep_thread.c. this tool is very very important,it is used to keep your token alive
  because if you do not run this tool ,your token will be invalid after more or less ten minutes.
  there are two ways to run this tool
  choice 1: ./token_keep_thread
  or you can use the second way
  choice 2: nohup ./token_keep_thread & (the tool will run in backgroud. Note: keep in mind ,after you umount fufs,you should kill this
   process.)
  
  if you want to know more,you can read the sina open API 
  here is the website:
  http://vdisk.me/api/doc
  
How to umount fufs from your linux desktop?
===================================================================
 umount:
  (if you do not want to use this filesystem,you can umount it.)
  fusermount -u /tmp/fufs

  if you run the small tools as the second way
  you should remember to kill the process
  kill -9 #(process number)
  
  
 Where can I get more information:
=====================================================
 Still have problems please send email to handsomestone@gmail.com
 
 http://blog.csdn.net/hiphopmattshi/article/details/7849408