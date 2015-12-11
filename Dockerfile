FROM alpine:3.2

ADD ./.output/acqueduct /usr/local/bin/acqueduct
CMD ["acqueduct", "-s"]
