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
strong_psp_b2_list = [2047, 3277, 4033, 4681, 8321, 15841, 29341, 42799, 49141, 52633, 65281, 74665, 80581, 85489, 88357, 90751, 104653, 130561, 196093, 220729, 233017, 252601, 253241, 256999, 271951, 280601, 314821, 357761, 390937, 458989, 476971, 486737]

-- List of counter examples to the PSW conjecture
counter_examples :: [Integer]
counter_examples = filter isFiboCounterExample strong_psp_b2_list

main :: IO ()
main = putStrLn $
    "Counter Examples to the PSW conjecture: " ++
    show counter_examples ++
    " (" ++ show (length counter_examples) ++ " found)"

