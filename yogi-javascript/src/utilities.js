window.yogi.nextTick = function(fn) {
  Promise.resolve().then(fn);
};
