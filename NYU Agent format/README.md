# NYU CTF Bench

This repository hosts the original NYU CTF Bench setting up files, and the formatted TribeCTF 2025 challenges.

## Benchmark structure

It keeps the original NYU_CTF_Bench structure (https://github.com/NYU-LLM-CTF/NYU_CTF_Bench). 

Additionally, we registered a TribeCTF dataset in `competition` folder. In each challenge folder the 'challenge.json' shows the name, category, description, required files, value and flag of the challenge. And the correctness will be checked automatically by NYU Autonomous Agent.

## Setup 

1. Download the dataset, and you can find details of all 17 challenges in `competition25.json`.
2. Launch the NYU CTF Agent
3. Use the command below to test our challenges:

```
python3 run_dcipher.py --split <competition> --challenge <challenge-name> [--enable-autoprompt]
```

## Remote Challenge (for TribeCTF 2025) Setup
There are three remote challenges, `gopr`, `dig me up`, and `not so top secret vault`.

For these challenges, please run the server first, and then launch NYU Agent to solve it.

The remote challenges setup instructions on local device can be found in `README-local.md` in `remote_TriveCTF2025_challenge` folder. It's better to use two devices or two separate environments on the same device.


## Contact

If you have any questions, please feel free to contact ttang01@wm.edu or yxiao05@wm.edu.


## Reference
```bibtex
@inproceedings{shao2024nyuctfbench,
     author = {Shao, Minghao and Jancheska, Sofija and Udeshi, Meet and Dolan-Gavitt, Brendan and xi, haoran and Milner, Kimberly and Chen, Boyuan and Yin, Max and Garg, Siddharth and Krishnamurthy, Prashanth and Khorrami, Farshad and Karri, Ramesh and Shafique, Muhammad},
     booktitle = {Advances in Neural Information Processing Systems},
     pages = {57472--57498},
     title = {NYU CTF Bench: A Scalable Open-Source Benchmark Dataset for Evaluating LLMs in Offensive Security},
     url = {https://proceedings.neurips.cc/paper_files/paper/2024/file/69d97a6493fbf016fff0a751f253ad18-Paper-Datasets_and_Benchmarks_Track.pdf},
     volume = {37},
     year = {2024}
}
