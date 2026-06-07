FROM golang:1.23-alpine AS builder

WORKDIR /app
COPY main.go .
COPY go.mod .

RUN go build -o main .

FROM scratch

COPY --from=builder /app/main .
COPY flag.txt .
COPY readme.txt .

EXPOSE 8080

CMD ["./main"]