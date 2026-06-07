# Ready, Set, Go!

## Difficulty
- **Easy**

## Category
- **Web, TOCTOU**

## Description
This challenge has a race condition bug due to reuse of `err` variable. File: `src/main.go` implements a rudimentary http server that takes `file_name` as a query param and replies with the contents of `file_name`.
```go
func main() {
	var err error // <-- err declared here
    // skipped
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
        // skipped
		err = isFileAllowed(filename) // <-- same `err` gets reused in all goroutines spawned by incoming requests
		fmt.Printf("[DEBUG] is %s allowed? %t\n", filename, err == nil)
		if err != nil {
			http.Error(w, err.Error(), http.StatusForbidden)
			return
        // skipped
```

The `isFileAllowed(filename)` throws an error if a user tries to read `flag.txt`.
```go
func isFileAllowed(file string) error {
	file = strings.TrimSpace(file)
	if file == "" {
		return fmt.Errorf("file name cannot be empty")
	}
	_, filename := filepath.Split(file)
	if strings.Contains(filename, "flag.txt") {
		return fmt.Errorf("access to flag.txt is not allowed")
	}
	return nil
}
```

On reading the file: `flag.txt`, `isFileAllowed()` method returns an error. The immediately following `fmt.Printf()` statement adds some delay before checking the `err` variable and returning an error. During this delay, let's say another goroutine reads a valid file (like `readme.txt`) and sets the `err` variable to `nil`. Once the first goroutine (which was reading the: `flag.txt`) resumes it doesn't see any errors as the `err` was reset to `nil` by another goroutine.
```go
    var err error // <-- err declared here
    // skipped
    http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
        // skipped
        err = isFileAllowed(filename) // <-- same `err` gets reused in all goroutines spawned by incoming requests
        fmt.Printf("[DEBUG] is %s allowed? %t\n", filename, err == nil) // <-- adds some delay 
        if err != nil {
            http.Error(w, err.Error(), http.StatusForbidden)
            return
        // skipped
```

## Limitations
- There's no session handling (or separation of user requests) currently, so a user can get lucky and receive a `nil` error while reading the flag. 

## Build & Run
```bash
# build the docker image
docker build -f src/Dockerfile -t ready-set-go src/

# atosh502/not-public is a private repository
# tag name is the name of challenge
docker build --platform linux/amd64 -f src/Dockerfile -t atosh502/not-public:ready-set-go src/
docker push atosh502/not-public:ready-set-go

# run the docker image
# docker run --cpus="0.1" --memory="8m" -p 8080:8080 ready-set-go
docker run -p 8080:8080 ready-set-go
```

## Flag
```bash
tribectf{n3v3r_5h4r3_y0ur_3rr_v4r14bl35}
```

## Solution
- First run the [build & run](#build--run) commands to start the problem
- Then run,
    ```bash
    # setup venv
    python3 -m venv venv
    . venv/bin/activate
    pip3 install requests

    cd solution
    python3 solve.py >> solve.txt
    cd -
    ```

## Appendix
- This challenge is based on the video: [Do you know this common Go vulnerability?](https://www.youtube.com/watch?v=wVknDjTgQoo) by LiveOverflow
- Original challenge writeup: [hxp 38C3 CTF: Fajny Jagazyn Wartości Kluczy](https://hxp.io/blog/114/hxp-38C3-CTF-Fajny-Jagazyn-Wartoci-Kluczy/)

## FAQ
- How to verify the race condition exists?
    ```bash
    # run the src/main.go with
    cd src/
    go run -race main.go

    # run the solution/solve.py with 
    cd solution/
    python3 solve.py

    # go race detector detects the race condition
    # ==================
    # WARNING: DATA RACE
    # Write at 0x00c000010100 by goroutine 15:
    #   main.main.func1()
    #       /Users/apoudel01/Documents/tribectf/TribeCTF-Challenges/2025/oc_challenges/ready_set_go/src/main.go:37 +0x138
    #   ...

    # Previous read at 0x00c000010100 by goroutine 10:
    #   main.main.func1()
    #       /Users/apoudel01/Documents/tribectf/TribeCTF-Challenges/2025/oc_challenges/ready_set_go/src/main.go:39 +0x22c
    #   ...

    # Goroutine 15 (running) created at:
    #   net/http.(*Server).Serve()
    #       /usr/local/go/src/net/http/server.go:3360 +0x674
    #   ...

    # Goroutine 10 (running) created at:
    #   net/http.(*Server).Serve()
    #       /usr/local/go/src/net/http/server.go:3360 +0x674
    #   ...
    # ==================
    ```


