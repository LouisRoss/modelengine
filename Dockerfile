FROM    louisross/modelengine-dev:1.0
LABEL   maintainer="Louis Ross <louis.ross@gmail.com"

ARG     MYDIR=/home/modelengine
WORKDIR ${MYDIR}

CMD     ["bash"]