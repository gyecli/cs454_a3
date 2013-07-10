//////////////////////////////////////////////////////////////////////////////////////////
// Extract from received （argTypes + args) combination from server
// Assuming the argTypes & args are from rpcCall() 
/*
void extract_args(char *buffer, int *argTypes, void** args) {

    int length = getTypeLength(argTypes);   // in bytes

    char* it = buffer + length;  

    int j = 0; 
    while(argTypes[i] != 0) {          // last element of argTypes is always ZERO
        // Type_mask = (255 << 16)
        unsigned int current_type = ((argTypes[j]) & Type_mask) >> 16; 
        unsigned int num = ((argTypes[j]) & array_size_mask);  // # of current arg of current_type
        int flag = 0; 
        if (num == 0) {
            num = 1; 
            flag = 1; 
        }

        switch(current_type) {
            case ARG_CHAR:
                // type: char
                char *temp2 = new char[num];
                memcpy(temp2, it, len);
                if (flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += num; 
                break; 

            case ARG_SHORT:
                // type: short
                short *temp2 = new short[num]
                memcpy (temp2, it, 2*num); 
                if(flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += 2*num; 
                break;
            case ARG_INT:
                // type: int
                short *temp2 = new short[num]
                memcpy (temp2, it, 4*num); 
                if(flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += 4*num; 
                break;
            case ARG_LONG:
                // type: long
                short *temp2 = new short[num]
                memcpy (temp2, it, 4*num); 
                if(flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += 4*num; 
                break;
            case ARG_DOUBLE:
                // type: double
                short *temp2 = new short[num]
                memcpy (temp2, it, 8*num); 
                if(flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += 8*num; 
                break; 
            case ARG_FLOAT:
                // type: float
                short *temp2 = new short[num]
                memcpy (temp2, it, 4*num); 
                if(flag == 1) {
                    args[j] = (void *) &(*temp2); 
                } else {
                    args[j] = (void *) temp2; 
                }
                it += 4*num; 
                break;
            default:
                break;
        }
        j++; 
    }
}
*/


//////////////////////////////////////////////////////////////////////////////////////////
/*void receiveMsg(int msgLen, int msgType, int sockfd) {
    
    int numbytes = recv(sockfd, rcv_buffer, msgLen, 0);
    if (numbytes <= 0) {
        if (numbytes == 0) {
            cout << "Target server is closed" << endl;
        } else {
            cerr << "ERROR in recving LOC reply from Binder" << endl;
        }
    } 

    if (msgType == LOC_SUCCESS) {
        // Received msg type: LOC_SUCCESS from Binder
        char rcv_buffer[msgLen];
        if (recv(sockfd, rcv_buffer, msgLen, 0) < 0) {
            cerr << "ERROR in recving LOC reply from Binder" << endl;
        } else {
            memcpy(server_id, rcv_buffer, 128); 
            memcpy(server_port, rcv_buffer+128, 2); 

            close(sockfd);          // finish receiving data from Binder
        }
    } else if (msgType == EXECUTE_SUCCESS) {
        // Received msg type: EXECUTE_SUCCESS from Server
        char rcv_buffer[msgLen];
        if (recv(sockfd, rcv_buffer, , 0) < 0) {
            cerr << "ERROR in recving LOC reply from Binder" << endl;
        } else {
            memcpy(rcv_name, rcv_buffer, 100);
            memcpy(rcv_argTypes, rcv_buffer, )
            ///////////////////////
            // need implement here
            ///////////////////////

            close(sockfd);          // finish receiving data from Binder
        }
    } else if (msgType == 0) {
        // Received msg type: FAILURE
    } 
}*/

