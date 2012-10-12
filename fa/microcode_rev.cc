/*
 * Copyright (C) 2012  Ondrej Lengal
 *
 * This file is part of forester.
 *
 * forester is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * forester is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with forester.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file microcode_rev.cc
 * Reverse operations for microcode instructions.
 */

// Forester headers
#include "microcode.hh"

SymState* FI_acc_sel::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)execMan;
	(void)fwdPred;
	(void)bwdSucc;

	assert(false);
}

SymState* FI_acc_set::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)execMan;
	(void)fwdPred;
	(void)bwdSucc;

	assert(false);
}

SymState* FI_acc_all::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)execMan;
	(void)fwdPred;
	(void)bwdSucc;

	assert(false);
}

SymState* FI_pop_greg::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)execMan;
	(void)fwdPred;
	(void)bwdSucc;

	assert(false);
}

SymState* FI_push_greg::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)execMan;
	(void)fwdPred;
	(void)bwdSucc;

	assert(false);
}

SymState* FI_abort::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)execMan;
	(void)fwdPred;
	(void)bwdSucc;

	assert(false);
}

SymState* FI_node_free::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)execMan;
	(void)fwdPred;
	(void)bwdSucc;

	assert(false);
}

SymState* FI_node_create::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)execMan;
	(void)fwdPred;
	(void)bwdSucc;

	assert(false);
}

SymState* FI_store::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)execMan;
	(void)fwdPred;
	(void)bwdSucc;

	assert(false);
}

SymState* FI_stores::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)execMan;
	(void)fwdPred;
	(void)bwdSucc;

	assert(false);
}

SymState* FI_set_greg::reverseAndIsect(
	ExecutionManager&                      execMan,
	const SymState&                        fwdPred,
	const SymState&                        bwdSucc) const
{
	(void)execMan;
	(void)fwdPred;
	(void)bwdSucc;

	assert(false);
}
