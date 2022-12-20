{-
This script tries to find counter-examples to the PSW conjeture by iterating
over the string pseudo-primes to base 2 (see haskell sources n°1).
-}

-- Some Constants
sq5 :: Double
sq5 = sqrt 5

phi :: Double
phi = (1 + sq5) / 2

psi :: Double
psi = (1 - sq5) / 2

-- Nth Fibonacci number
fibo :: Integer -> Integer
fibo n = round $ ((phi ^^ n) - (psi ^^ n)) / sq5

-- Check if the input is a counter example to the PSW conjecture
-- The input should already be a strong pseudoprime to base 2
isFiboCounterExample :: Integer -> Bool
isFiboCounterExample n = (fibo (n + 1) `mod` n) == 0

-- List of Strong PseudoPrimes to Base 2 (see haskell sources n°1).
strong_psp_b2_list :: [Integer]
strong_psp_b2_list = [341,561,645,1105,1387,1729,1905,2047,2465,2701,2821,3277,4033,4369,4371,4681,5461,6601,7957,8321,8481,8911,10261,10585,11305,12801,13741,13747,13981,14491,15709,15841,16705,18705,18721,19951,23001,23377,25761,29341]

-- List of counter examples to the PSW conjecture
counter_examples :: [Integer]
counter_examples = filter isFiboCounterExample strong_psp_b2_list

main :: IO ()
main = putStrLn $
    "Counter Examples to the PSW conjecture: " ++
    show counter_examples ++
    " (" ++ show (length counter_examples) ++ " found)"

