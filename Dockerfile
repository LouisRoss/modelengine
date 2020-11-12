FROM    ubuntu:20.10
LABEL   maintainer="Louis Ross <louis.ross@gmail.com"

ARG     MYDIR=/home/modelengine
WORKDIR ${MYDIR}

COPY    install-deps ${MYDIR}/
#COPY    googletest ${MYDIR}/googletest/

RUN     echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections
RUN     bash ${MYDIR}/install-deps >>install-deps.log
CMD     ["bash"]