dbadmin release v1.0.1

----------------- RELEASE VERSION INFO -----------

  rev:		notes:
  -------------------
  ALPHA		- Product under development. Little usable features. Many Bugs.
  BETA		- First release to general public. More features than bugs.
  v1.0		- Most functionality exists. Some known bugs, none destructive.
  v1.0.1	- NULL handling is much improved. When inserting character data
                to a table, a NULL value may be specified by leaving the entry
                blank, or by specifying "(null)" as the value. NULL int and
		real fields should be left blank.
                - Makefile modified. Many users could not complile easily.



----------------- RELEASE TERMS ------------------
This is the version 1.0.x release of the dbadmin toolkit. The code now contains
more features that bugs. However, the standard disclaimer applies: I make no
claims as to the usability or functionality of this product for any particular
use. USE AT YOUR OWN RISK. 

This is NOT being released under a public license.  If you intend to use this
program at a commercial site, you must register your copy and pay the license
fee. If you use this program otherwise and desire updates and bug fixes, please
send me email with your "non-commercial" registration. It is likely that
updates will be sent to all registered users by email.



----------------- COMPILE NOTES -------------------
To compile the executable in a manner compatible with your specific
configuration, a few minor changes to the Makefile and dbadmin.h are
necessary. You will need 2 very important pieces of information:

 1) Absolute path where the CGI should be installed.
    - Change the Makefile variable "CGIDIR" to reflect this absolute path.
 2) Path relative to the httpd-root where it will be accessed.
    - Change "dbadmin.h" #define RELPATH to reflect the httpd's perspective
      of where the CGI will live as a URL.

 NEXT COMPILE STEPS ARE:

 3) Copy or link your "msql.h" into the build directory.
 4) type "make"


----------------- INSTALL NOTES ---------------------
For the CGI to have mSQL permissions to create and remove databases,
it must run under the username which is the mSQL owner. Thus it must
run setuid (remember httpd's run the CGI's under a particular username,
which is nearly guaranteed NOT to be the mSQL owner). If setuid programs
are not allowed on your system, you may have trouble here.

1) "su" to the mSQL owner ID
2) type "make install" to put the cgi in the right location, setuid
3) make a bookmark to "/cgi-bin/dbadmin/dbadmin" (or whatever you changed
   this to)
4) password protect that directory! (use a .htaccess file!!!!!)
5) ensure that the directory is "ScriptAliased" in the httpd configurations



------------------ OTHER AREAS OF INTEREST -------------

Currently, the "CONNECT SERVER" option is non-functional. I left it there
to entice myself into getting it done; and to entice users into suggesting
to me the correct methods of doing that... If you have a good solution there,
please let me know... Some issues to be overcome:

1) User ID
2) UsEr Id
3) USer iD
etc.

Exactly what "server" does that specify; A different httpd server, or a
different mSQL server... 

------------------ CREDITS STUFF -----------------------

MSQL: what do I need to say here? Good work, Bambi. The first license fees
I collect will go straight to the Man himself. 

The CGI lobrary included with the program is copyrighted ...

>cgic License Terms
>------------------
>
>Basic License
>-------------
>
>cgic, copyright 1996 by Thomas Boutell. Permission is granted to use cgic 
>in any application, commercial or noncommercial, at no cost. HOWEVER,
>this copyright paragraph must appear on a "credits" page accessible in 
>the public online and offline documentation of the program. Modified 
>versions of the cgic library should not be distributed without the 
>attachment of a clear statement regarding the author of the 
>modifications, and this notice may in no case be removed. 
>Modifications may also be submitted to the author for inclusion 
>in the main cgic distribution.                                 

Done. If several people register their distributions, I will be able to send
this guy a well deserved check for the ability to remove that credits notice.
(Excellent package BTW)


-------------- MOST IMPORTANTLY ----------------

PLEASE SEND ME FEEDBACK ON THE USE OF THIS PRODUCT! I hope to be actively
updating it, as the primary reason I developed it was because I needed it!
I figured you might want it as well...

James E. Harrell, Jr.
April 23, 1996
jharrell@emi.net

