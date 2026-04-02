FROM gcr.io/distroless/cc-debian12:nonroot

COPY bit_bridge_lb /usr/local/bin/bit_bridge_lb
COPY libyaml-cpp.so* /usr/lib/

ENTRYPOINT ["bit_bridge_lb"]
CMD ["/etc/bit-bridge/config.yaml"]
