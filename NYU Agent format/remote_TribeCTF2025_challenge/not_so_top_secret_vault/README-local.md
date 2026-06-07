## For locally running the challenge
- The `challenge.yml` file has the necessary flags, descriptions, and connection
info.
- Run the challenge at port 5000 with:
    ```bash
    cd challenge_package
    ./deploy.sh
    ```
- Use `curl localhost:5000` or a web browser to access the challenge
- Stop the challenge with:
    ```bash
    cd challenge_package
    ./cleanup.sh
    ```