{-
This program finds all the counter-examples to the PSW conjeture
using a list of all the Fermat pseudo-primes to base 2 that are
less than 10^12.

(see haskell sources n°1).
-}

-- Nth Fibonacci number (F0 = ?, F1 = 1, F2 = 1, F3 = 2)
fibo :: Integer -> Integer
fibo = fst . fib_ . pred
    where
        fib_ 0 = (1, 1)
        fib_ 1 = (1, 2)
        fib_ n
            | even n    = (a*a + b*b, c*c - a*a)
            | otherwise = (c*c - a*a, b*b + c*c)
            where
                (a,b) = fib_ (n `div` 2 - 1)
                c     = a + b

-- Check if the input is a counter example to the PSW conjecture
-- The input should already be a strong pseudoprime to base 2
isFiboCounterExample :: Integer -> Bool
isFiboCounterExample n = (fibo (n + 1)) `mod` n == 0

-- Must have the right remainder
isRightMod5 :: Integer -> Bool
isRightMod5 = (\n -> n == 2 || n == 3) . (flip rem 5)

-- List of Fermat pseudo-prime numbers to base 2 (see haskell sources n°1).
strong_psp_b2_list :: IO [Integer]
strong_psp_b2_list = do
    content <- readFile "pseudo-primes-b2.txt"
    return $ map (read . tail . (dropWhile (/=' '))) (lines content)

-- List of numbers that are -2 (or 3) or 2 mod 5
selected_strong_psp_b2_list :: IO [Integer]
selected_strong_psp_b2_list = (filter isRightMod5) <$> strong_psp_b2_list

-- List of counter examples to the PSW conjecture
counter_examples :: IO [Integer]
counter_examples = (filter isFiboCounterExample) <$> selected_strong_psp_b2_list

main :: IO ()
main = do
    pure_counter_examples <- counter_examples
    putStrLn $
        "Counter Examples to the PSW conjecture: " ++
        show pure_counter_examples ++
        " (" ++ show (length pure_counter_examples) ++ " found)"

