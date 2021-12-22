## History

### One or many executors?

Should a given `History` instance be able to be used with different types of `Executor`s? Or should we instead force users to declare during construction the `Executor` that can be used with a given history?

I think we will opt for generality for now, and see if anything bad happens. (NB: I guess we could also have both types of `History`).

## Executor

We split the interface into `Executor` and `Reverter` because of ISP (the Interface Segregation Principle from SOLID).