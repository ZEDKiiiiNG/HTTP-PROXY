From gcc:latest

COPY . /src/
RUN mkdir -p /var/src/erss
RUN touch /var/src/erss/proxy.log
WORKDIR /src

RUN apt-get update && apt-get install -y make && apt-get install -y g++

RUN chmod +x run.sh
ENTRYPOINT ["/bin/sh", "./run.sh"]

LABEL Name=httpcachingproxy Version=0.0.1
