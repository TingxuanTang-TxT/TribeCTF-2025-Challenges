### Commands
```bash
# build the image (inside DuckHunt-JS-K8s)
docker build -t ducker-hunt .
docker build --secret id=ducked,src=../flag.txt -t ducker-hunt .

# save image as .tar file
docker save -o ducker-hunt.tar ducker-hunt

# run the saved .tar image file
docker load -i ducker-hunt.tar

# run the image
docker run -p 8080:8080 ducker-hunt

# inspect image with dive
# inspect layers (the third last layer/dir has the flag.txt file)
docker image inspect ducker-hunt | jq '.[].GraphDriver.Data.UpperDir + ":" + .[].GraphDriver.Data.LowerDir | split(":") | reverse'
```

- Flag is `base64` encoded in order to avoid grepping flags from docker image with `strings`

References
- [Using `docker inspect` and `dive`](https://danaepp.com/finding-api-secrets-in-hidden-layers-within-docker-containers)
- [Docker image packed at .tar file](https://gemini.google.com/app/dcf1b9cf310cf4da)
- [`dive` docs](https://github.com/wagoodman/dive)
- [inspect local docker image](https://github.com/wagoodman/dive/issues/360)
- [duck hunt game](https://github.com/nutta3/DuckHunt-JS-K8s)