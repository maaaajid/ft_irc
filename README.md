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
