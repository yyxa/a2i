import subprocess
import unittest
import os
import shutil

class TestCLIAudioFlags(unittest.TestCase):
    def setUp(self):
        self.exe   = "./cli/build/a2i"
        self.audio = "music/trip.mp3"
        cfg = os.path.expanduser("~/.config/a2i")
        if os.path.isdir(cfg):
            shutil.rmtree(cfg)

    def run_audio(self, flags, expect_success=True):
        try:
            result = subprocess.run(
                [self.exe, self.audio] + flags,
                input="q\n",
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                timeout=5
            )
        except subprocess.TimeoutExpired:
            self.fail(f"Process hung with flags {flags!r}")
        if expect_success:
            self.assertEqual(result.returncode, 0,
                msg=f"Expected success but got returncode={result.returncode}, stderr={result.stderr}")
        else:
            self.assertNotEqual(result.returncode, 0,
                msg=f"Expected failure but got returncode=0 for flags {flags!r}")
        return result

    #
    # Numeric flags: valid / invalid boundaries
    #
    # framesize ≥512 and power of two
    def test_framesize(self):
        self.run_audio(["--framesize=512"], expect_success=True)
        self.run_audio(["--framesize=1024"], expect_success=True)
        self.run_audio(["--framesize=2048"], expect_success=True)
        self.run_audio(["--framesize=4096"], expect_success=True)
        self.run_audio(["--framesize=8192"], expect_success=True)
        self.run_audio(["--framesize=16384"], expect_success=True)
        self.run_audio(["--framesize=32768"], expect_success=True)
        self.run_audio(["--framesize=65536"], expect_success=True)
        self.run_audio(["--framesize=131072"], expect_success=True)
        
        self.run_audio(["--framesize=256"], expect_success=False)
        self.run_audio(["--framesize=511"], expect_success=False)
        self.run_audio(["--framesize=513"], expect_success=False)
        self.run_audio(["--framesize=1025"], expect_success=False)
        
        self.run_audio(["--framesize=."], expect_success=False)
        self.run_audio(["--framesize=a"], expect_success=False)
        self.run_audio(["--framesize=1.5"], expect_success=False)
        self.run_audio(["--framesize=1,5"], expect_success=False)
        self.run_audio(["--framesize="], expect_success=False)

    # window-func ∈ [0,9]
    def test_window_func(self):
        self.run_audio(["--window-func=0"], expect_success=True)
        self.run_audio(["--window-func=1"], expect_success=True)
        self.run_audio(["--window-func=2"], expect_success=True)
        self.run_audio(["--window-func=3"], expect_success=True)
        self.run_audio(["--window-func=4"], expect_success=True)
        self.run_audio(["--window-func=5"], expect_success=True)
        self.run_audio(["--window-func=6"], expect_success=True)
        self.run_audio(["--window-func=7"], expect_success=True)
        self.run_audio(["--window-func=8"], expect_success=True)
        self.run_audio(["--window-func=9"], expect_success=True)
        
        self.run_audio(["--window-func=-1"], expect_success=False)
        self.run_audio(["--window-func=10"], expect_success=False)
        
        self.run_audio(["--window-func=."], expect_success=False)
        self.run_audio(["--window-func=a"], expect_success=False)
        self.run_audio(["--window-func=1.5"], expect_success=False)
        self.run_audio(["--window-func=1,5"], expect_success=False)
        self.run_audio(["--window-func="], expect_success=False)

    # line-type ∈ [0,2]
    def test_line_type(self):
        self.run_audio(["--line-type=0"], expect_success=True)
        self.run_audio(["--line-type=1"], expect_success=True)
        self.run_audio(["--line-type=2"], expect_success=True)
        
        self.run_audio(["--line-type=-1"], expect_success=False)
        self.run_audio(["--line-type=3"], expect_success=False)
        
        self.run_audio(["--line-type=."], expect_success=False)
        self.run_audio(["--line-type=a"], expect_success=False)
        self.run_audio(["--line-type=1.5"], expect_success=False)
        self.run_audio(["--line-type=1,5"], expect_success=False)
        self.run_audio(["--line-type="], expect_success=False)

    # graph-mode ∈ [0,1]
    def test_graph_mode(self):
        self.run_audio(["--graph-mode=0"], expect_success=True)
        self.run_audio(["--graph-mode=1"], expect_success=True)
        
        self.run_audio(["--graph-mode=-1"], expect_success=False)
        self.run_audio(["--graph-mode=2"], expect_success=False)
        
        self.run_audio(["--graph-mode=."], expect_success=False)
        self.run_audio(["--graph-mode=a"], expect_success=False)
        self.run_audio(["--graph-mode=1.5"], expect_success=False)
        self.run_audio(["--graph-mode=1,5"], expect_success=False)
        self.run_audio(["--graph-mode="], expect_success=False)

    # previous-frames ≥ 0
    def test_previous_frames_valid(self):
        self.run_audio(["--previous-frames=0"], expect_success=True)
        self.run_audio(["--previous-frames=5"], expect_success=True)
        self.run_audio(["--previous-frames=10"], expect_success=True)
        self.run_audio(["--previous-frames=20"], expect_success=True)
        
        self.run_audio(["--previous-frames=-3"], expect_success=False)
        self.run_audio(["--previous-frames=-1"], expect_success=False)
        self.run_audio(["--previous-frames=2"], expect_success=False)
        
        self.run_audio(["--previous-frames=."], expect_success=False)
        self.run_audio(["--previous-frames=a"], expect_success=False)
        self.run_audio(["--previous-frames=1.5"], expect_success=False)
        self.run_audio(["--previous-frames=1,5"], expect_success=False)
        self.run_audio(["--previous-frames="], expect_success=False)
        
    # colormap ∈ [0,21]
    def test_colormap(self):
        self.run_audio(["--colormap=0"], expect_success=True)
        self.run_audio(["--colormap=1"], expect_success=True)
        self.run_audio(["--colormap=2"], expect_success=True)
        self.run_audio(["--colormap=3"], expect_success=True)
        self.run_audio(["--colormap=4"], expect_success=True)
        self.run_audio(["--colormap=5"], expect_success=True)
        self.run_audio(["--colormap=6"], expect_success=True)
        self.run_audio(["--colormap=7"], expect_success=True)
        self.run_audio(["--colormap=8"], expect_success=True)
        self.run_audio(["--colormap=9"], expect_success=True)
        self.run_audio(["--colormap=10"], expect_success=True)
        self.run_audio(["--colormap=11"], expect_success=True)
        self.run_audio(["--colormap=12"], expect_success=True)
        self.run_audio(["--colormap=13"], expect_success=True)
        self.run_audio(["--colormap=14"], expect_success=True)
        self.run_audio(["--colormap=15"], expect_success=True)
        self.run_audio(["--colormap=16"], expect_success=True)
        self.run_audio(["--colormap=17"], expect_success=True)
        self.run_audio(["--colormap=18"], expect_success=True)
        self.run_audio(["--colormap=19"], expect_success=True)
        self.run_audio(["--colormap=20"], expect_success=True)
        self.run_audio(["--colormap=21"], expect_success=True)

        self.run_audio(["--colormap=-1"], expect_success=False)
        self.run_audio(["--colormap=22"], expect_success=False)
        
        self.run_audio(["--colormap=."], expect_success=False)
        self.run_audio(["--colormap=a"], expect_success=False)
        self.run_audio(["--colormap=1.5"], expect_success=False)
        self.run_audio(["--colormap=1,5"], expect_success=False)
        self.run_audio(["--colormap="], expect_success=False)

    # fill-type ∈ [0,2]
    def test_fill_type(self):
        self.run_audio(["--fill-type=0"], expect_success=True)
        self.run_audio(["--fill-type=1"], expect_success=True)
        self.run_audio(["--fill-type=2"], expect_success=True)

        self.run_audio(["--fill-type=-1"], expect_success=False)
        self.run_audio(["--fill-type=3"], expect_success=False)
        
        self.run_audio(["--fill-type=."], expect_success=False)
        self.run_audio(["--fill-type=a"], expect_success=False)
        self.run_audio(["--fill-type=1.5"], expect_success=False)
        self.run_audio(["--fill-type=1,5"], expect_success=False)
        self.run_audio(["--fill-type="], expect_success=False)

    # colormap-coef ∈ [0,255]
    def test_colormap_coef(self):
        self.run_audio(["--colormap-coef=0"], expect_success=True)
        self.run_audio(["--colormap-coef=127"], expect_success=True)
        self.run_audio(["--colormap-coef=255"], expect_success=True)

        self.run_audio(["--colormap-coef=-1"], expect_success=False)
        self.run_audio(["--colormap-coef=256"], expect_success=False)
        
        self.run_audio(["--colormap-coef=."], expect_success=False)
        self.run_audio(["--colormap-coef=a"], expect_success=False)
        self.run_audio(["--colormap-coef=1.5"], expect_success=False)
        self.run_audio(["--colormap-coef=1,5"], expect_success=False)
        self.run_audio(["--colormap-coef="], expect_success=False)

    # normalize-multiplier ≥ 0
    def test_normalize_multiplier(self):
        self.run_audio(["--normalize-multiplier=1"], expect_success=True)
        self.run_audio(["--normalize-multiplier=10"], expect_success=True)
        self.run_audio(["--normalize-multiplier=20"], expect_success=True)
        self.run_audio(["--normalize-multiplier=40"], expect_success=True)

        self.run_audio(["--normalize-multiplier=0"], expect_success=False)
        self.run_audio(["--normalize-multiplier=-1"], expect_success=False)
        
        self.run_audio(["--normalize-multiplier=."], expect_success=False)
        self.run_audio(["--normalize-multiplier=a"], expect_success=False)
        self.run_audio(["--normalize-multiplier=1.5"], expect_success=False)
        self.run_audio(["--normalize-multiplier=1,5"], expect_success=False)
        self.run_audio(["--normalize-multiplier="], expect_success=False)

    # volume ∈ [0.0,1.0]
    def test_volume(self):
        self.run_audio(["--volume=0.0"], expect_success=True)
        self.run_audio(["--volume=0.25"], expect_success=True)
        self.run_audio(["--volume=0.5"], expect_success=True)
        self.run_audio(["--volume=1.0"], expect_success=True)

        self.run_audio(["--volume=-0.1"], expect_success=False)
        self.run_audio(["--volume=1.1"], expect_success=False)
        
        self.run_audio(["--volume=."], expect_success=False)
        self.run_audio(["--volume=a"], expect_success=False)
        self.run_audio(["--volume=1,5"], expect_success=False)
        self.run_audio(["--volume="], expect_success=False)

    #
    # Boolean flags (no value) should always succeed
    #
    def test_flag_border(self):
        self.run_audio(["--border"], expect_success=True)

    def test_flag_grid(self):
        self.run_audio(["--grid"], expect_success=True)

    def test_flag_onlyaudio(self):
        self.run_audio(["--onlyaudio"], expect_success=True)

    def test_flag_debug(self):
        self.run_audio(["--debug"], expect_success=True)
        
    def test_flag_random(self):
        self.run_audio(["--random"], expect_success=False)

if __name__ == "__main__":
    unittest.main()
