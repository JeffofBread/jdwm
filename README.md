# Jeff's dwm build

A build of dwm for myself and anyone who wishes to use it too. If you find any issues or would like to make a recommendation, feel free. This is a major work in progress and will be changing often and will be receiving major changes in the near future. Any recommendations or critique is appreciated.
## Installation for new users

```bash
git clone https://github.com/JeffofBread/jeff_dwm.git
cd jeff_dwm
sudo ./install.sh
```
After your first install, you can use `Ctrl + Shift + Q` to open the recompile script in kitty. To restart jeff_dwm ([assuming you haven't removed the auto-restart script in `jeff_dwm-run.sh`](https://github.com/JeffofBread/jeff_dwm/blob/e9e86bc5097c194a8f25db21be4e961054f4e123/dwm-src/scripts/jeff_dwm-run.sh#L19C1-L19C61)), press `Super + Shift + Q`.

## Dependencies

- yajl
- imlib2
- kitty (Heavily recommended and built on, but can be changed in source if you want to spend the time)

## Recommended

- picom
- feh
