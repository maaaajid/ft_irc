brahim :
    IRC structure :
        - src : contains .cpp files
            - channel : will contain a class Channel that handles channel operations (join, leave, kick, topics...)
            - client : will contain a Client class that handles client's infos and actions
            - command : will contain a Command class for commands functions
            - logger : will contain a Logger class for printing messages based on their importance level (INFO, WARNING, ERROR...)
            - server : will contain a Server class for server's initialization, multiplexing...
                - previous class "multiplexing" is now "Server"
            - utils : will contain helper functions for the project

        - includes : contains .hpp files
            - irc : will include all the other header files and will be the only header file called in cpp files
        
        main.cpp : main file for the project, checks for arguments validity and initializes the server





Tested Cases:

case1:  after lunching the server, i connect it  (/connect localhost port password) then i quit the server o kan7awel nlogi mera khera bla password lfd kaydeleta so luser makhassoch ylogi o hadche li kaywqee3.

``    ➜  ft_irc git:(main) ✗ ./ircserv 8080 1234
    [01:17:00] [INFO] Server Started !
    [01:17:00] [INFO] Server Ip: 127.0.0.1 , Port: 8080
    [01:17:00] [INFO] Waiting connections...
    buffer: CAP LS
    PASS 1234
    NICK mbouayou
    USER mbouayou mbouayou localhost :Mohamed Bouayoune

    cmds: CAP
    [01:18:05] [INFO] Deleted user: '' fd: 5
    buffer: CAP LS
    NICK mbouayou

    cmds: CAP
    buffer: USER mbouayou mbouayou localhost :Mohamed Bouayoune

    cmds: USER
``

case2: kayn problem f sockets server kay lost lconnection mea lclient (solution: lqit wahed leaiba PING\PONG kayblqa server ysifet chi mssg lclient bach mat losach). wela khass replies ikon bach katbqa lconnection i m not sure.



bro 

**khassni naddi allh replies to the client

 11-25
 baqi lia quit method (anclose lconnection  o remove luser men userlist) but lqit haja khera khessha tzad hia tnotify lusers lkhrin ila kan dakhel lchi channel bli quita . earfti kifsh nqder nqadha.

dok numeric rpl rah gha template gha nreglhom.
 
sinon testi dakchi lakhor ig rah mzn. o ela endek chi idea chi haja khassha tzad wela trigel gha sifet lia abro