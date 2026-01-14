# Changelog

## General

## lace

- Filter out certain `RefExpr`'s from being considered lvalues, e.g. anything
that isn't a parameter or variable reference.
- Restructured LIR code generation to use less state and more function 
delegation.

## LIR

- Add aggregate constant values.
