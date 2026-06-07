progs= exploit setup

all: $(progs)

$(progs): % : vet
	go build ./cmd/$@

vet: fmt
	go vet ./...

fmt:
	go fmt ./...
	
# -count=1 forces tests to always run, even if no code has changed
test: vet
	go test -v -vet=all -count=1 ./...

clean:
	rm -f $(progs)

.PHONY: all vet fmt test clean
