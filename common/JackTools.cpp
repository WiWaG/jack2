/*
  Copyright (C) 2001 Paul Davis

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "JackConstants.h"
#include "JackTools.h"
#include "JackError.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include <process.h>
#endif

using namespace std;

namespace Jack {

#define DEFAULT_TMP_DIR "/tmp"
    char* jack_tmpdir = (char*)DEFAULT_TMP_DIR;

    int JackTools::GetPID() {
#ifdef WIN32
        return  _getpid();
#else
        return getpid();
#endif
    }

    int JackTools::GetUID() {
#ifdef WIN32
        return  _getpid();
        //#error "No getuid function available"
#else
        return getuid();
#endif
    }

    const char* JackTools::DefaultServerName() {
        const char* server_name;
        if ((server_name = getenv("JACK_DEFAULT_SERVER")) == NULL)
            server_name = JACK_DEFAULT_SERVER_NAME;
        return server_name;
    }

    /* returns the name of the per-user subdirectory of jack_tmpdir */
#ifdef WIN32

    char* JackTools::UserDir() {
        return "";
    }

    char* JackTools::ServerDir(const char* server_name, char* server_dir) {
        return "";
    }

    void JackTools::CleanupFiles(const char* server_name) {}

    int JackTools::GetTmpdir() {
        return 0;
    }

#else
    char* JackTools::UserDir() {
        static char user_dir[PATH_MAX + 1] = "";

        /* format the path name on the first call */
        if (user_dir[0] == '\0') {
            if (getenv ("JACK_PROMISCUOUS_SERVER")) {
                snprintf(user_dir, sizeof(user_dir), "%s/jack", jack_tmpdir);
            } else {
                snprintf(user_dir, sizeof(user_dir), "%s/jack-%d", jack_tmpdir, GetUID());
            }
        }

        return user_dir;
    }

    /* returns the name of the per-server subdirectory of jack_user_dir() */
    char* JackTools::ServerDir(const char* server_name, char* server_dir) {
        /* format the path name into the suppled server_dir char array,
        * assuming that server_dir is at least as large as PATH_MAX+1 */

        snprintf(server_dir, PATH_MAX + 1, "%s/%s", UserDir(), server_name);
        return server_dir;
    }

    void JackTools::CleanupFiles(const char* server_name) {
        DIR* dir;
        struct dirent *dirent;
        char dir_name[PATH_MAX + 1] = "";
        ServerDir(server_name, dir_name);

        /* On termination, we remove all files that jackd creates so
        * subsequent attempts to start jackd will not believe that an
        * instance is already running. If the server crashes or is
        * terminated with SIGKILL, this is not possible. So, cleanup
        * is also attempted when jackd starts.
        *
        * There are several tricky issues. First, the previous JACK
        * server may have run for a different user ID, so its files
        * may be inaccessible. This is handled by using a separate
        * JACK_TMP_DIR subdirectory for each user. Second, there may
        * be other servers running with different names. Each gets
        * its own subdirectory within the per-user directory. The
        * current process has already registered as `server_name', so
        * we know there is no other server actively using that name.
        */

        /* nothing to do if the server directory does not exist */
        if ((dir = opendir(dir_name)) == NULL) {
            return;
        }

        /* unlink all the files in this directory, they are mine */
        while ((dirent = readdir(dir)) != NULL) {

            char fullpath[PATH_MAX + 1];

            if ((strcmp(dirent->d_name, ".") == 0) || (strcmp (dirent->d_name, "..") == 0)) {
                continue;
            }

            snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_name, dirent->d_name);

            if (unlink(fullpath)) {
                jack_error("cannot unlink `%s' (%s)", fullpath, strerror(errno));
            }
        }

        closedir(dir);

        /* now, delete the per-server subdirectory, itself */
        if (rmdir(dir_name)) {
            jack_error("cannot remove `%s' (%s)", dir_name, strerror(errno));
        }

        /* finally, delete the per-user subdirectory, if empty */
        if (rmdir(UserDir())) {
            if (errno != ENOTEMPTY) {
                jack_error("cannot remove `%s' (%s)", UserDir(), strerror(errno));
            }
        }
    }

    int JackTools::GetTmpdir() {
        FILE* in;
        size_t len;
        char buf[PATH_MAX + 2]; /* allow tmpdir to live anywhere, plus newline, plus null */

        if ((in = popen("jackd -l", "r")) == NULL) {
            return -1;
        }

        if (fgets(buf, sizeof(buf), in) == NULL) {
            fclose(in);
            return -1;
        }

        len = strlen(buf);

        if (buf[len - 1] != '\n') {
            /* didn't get a whole line */
            fclose(in);
            return -1;
        }

        jack_tmpdir = (char *)malloc(len);
        memcpy(jack_tmpdir, buf, len - 1);
        jack_tmpdir[len - 1] = '\0';

        fclose(in);
        return 0;
    }
#endif

    void JackTools::RewriteName(const char* name, char* new_name) {
        size_t i;
        for (i = 0; i < strlen(name); i++) {
            if ((name[i] == '/') || (name[i] == '\\'))
                new_name[i] = '_';
            else
                new_name[i] = name[i];
        }
        new_name[i] = '\0';
    }

    JackArgParser::JackArgParser(const char* arg) {
        jack_log ( "JackArgParser::JackArgParser, arg : '%s'", arg );

        fArgc=0;
        fNumArgv=0;
        fArgString=string(arg);
        //if empty string
        if (strlen(arg)==0) {
            fArgv = NULL;
            return;
        }
        //else parse the arg string
        const size_t arg_len=fArgString.length();
        int i=0;
        size_t pos=0;
        size_t start=0;
        size_t copy_start=0;
        size_t copy_length=0;
        vector<string> args;
        //we need a 'space terminated' string
        fArgString+=" ";
        //first fill a vector with args
        do {
            //find the first non-space character from the actual position
            start=fArgString.find_first_not_of(' ',start);
            //get the next quote or space position
            pos=fArgString.find_first_of(" \"",start);
            //no more quotes or spaces, consider the end of the string
            if (pos==string::npos)
                pos=arg_len;
            //if double quote
            if (fArgString.at(pos)=='\"') {
                //first character : copy the substring
                if (pos==start) {
                    copy_start=start+1;
                    pos=fArgString.find('\"',++pos);
                    copy_length=pos-copy_start;
                    start=pos+1;
                }
                //else there is someting before the quote, first copy that
                else {
                    copy_start=start;
                    copy_length=pos-copy_start;
                    start=pos;
                }
            }
            //if space
            if (fArgString.at(pos)==' ') {
                copy_start=start;
                copy_length=pos-copy_start;
                start=pos+1;
            }
            //then push the substring to the args vector
            args.push_back(fArgString.substr(copy_start,copy_length));
        } while(start<arg_len);

        //and then duplicate args into the argv array
        fNumArgv=args.size();
        fArgv=new char* [fNumArgv];
        for (i=0; i<fNumArgv; i++) {
            fArgv[i]=new char[args[i].length()];
            fill_n(fArgv[i],args[i].length()+1,0);
            args[i].copy(fArgv[i],args[i].length());
        }

        //finally count the 'real' options (the ones starting with a '-')
        for (i=0; i<fNumArgv; i++)
            if (fArgv[i][0]=='-')
                fArgc++;
    }

    JackArgParser::~JackArgParser() {
        for (int i=0; i<fNumArgv; i++)
            delete[] fArgv[i];
        delete[] fArgv;
    }

    string JackArgParser::GetArgString() {
        return fArgString;
    }

    int JackArgParser::GetNumArgv() {
        return fNumArgv;
    }

    int JackArgParser::GetArgc() {
        return fArgc;
    }

    const char** JackArgParser::GetArgv() {
        return const_cast<const char**>(fArgv);
    }
}

