describe('Utilities', () => {
  describe('nextTick', () => {
    it('should execute the given function in the next JS engine tick', (done) => {
      let x = [];
      yogi.nextTick(() => {
        x.push(123);
        done();
      });
      expect(x.length).toBe(0);
    });
  });
});
