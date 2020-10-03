#!/bin/sh
#
#
##################################################
# Customize this section to meet your needs
##################################################
PROD=GMAK
SPR=${PROD}V03

# Where to put files 

NSK_HOST=15.199.36.66
NSK_USER_ID=hpiso1.kannanmj
DEST_SUBVOL=fsdev1.${SPR}


##################################################
# End of customization section
##################################################

# Transfer everything to the submission subvolume      

ftp -n -i ${NSK_HOST} <<FTP_EOF
    user ${NSK_USER_ID}
    quote guardian
    verbose
    ascii
    cd \$${DEST_SUBVOL}
    put bldgmak bldgmak,101,14,28
    put bldgmake bldgmake,101,14,28
    put makein makein,101,14,28
    put salloca.c sallocac,101,14,28
    put sar.c sarc,101,14,28
    put sarscan.c sarscanc,101,14,28
    put scommnd.c scommndc,101,14,28
    put sdefalt.c sdefaltc,101,14,28
    put sdir.c sdirc,101,14,28
    put sexpand.c sexpandc,101,14,28
    put sfile.c sfilec,101,14,28
    put sfnmach.c sfnmachc,101,14,28
    put sfunctn.c sfunctnc,101,14,28
    put sgetlod.c sgetlodc,101,14,28
    put sgetop1.c sgetop1c,101,14,28
    put sgetopt.c sgetoptc,101,14,28
    put sglob.c sglobc,101,14,28
    put sjob.c sjobc,101,14,28
    put simplic.c simplicc,101,14,28
    put smain.c smainc,101,14,28
    put sver.c sverc,101,14,28
    put smisc.c smiscc,101,14,28
    put sread.c sreadc,101,14,28
    put sremake.c sremakec,101,14,28
    put sremstb.c sremstbc,101,14,28
    put srule.c srulec,101,14,28
    put ssignam.c ssignamc,101,14,28
    put standem.c standemc,101,14,28
    put stanfun.c stanfunc,101,14,28
    put svariab.c svariabc,101,14,28
    put sverson.c sversonc,101,14,28
    put svpath.c svpathc,101,14,28
    put wcommnd.h wcommndh,101,14,28
    put wconfig.h wconfigh,101,14,28
    put wdep.h wdeph,101,14,28
    put wear.h wearh,101,14,28
    put wfildef.h wfildefh,101,14,28
    put wfnmach.h wfnmachh,101,14,28
    put wgetopt.h wgetopth,101,14,28
    put wglob.h wglobh,101,14,28
    put wjob.h wjobh,101,14,28
    put wmake.h wmakeh,101,14,28
    put wrule.h wruleh,101,14,28
    put wsignam.h wsignamh,101,14,28
    put wvariab.h wvariabh,101,14,28
    put wxar.h wxarh,101,14,28
   bye
FTP_EOF
