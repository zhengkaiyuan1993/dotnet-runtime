// Copyright (c) .NET Foundation and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

using System;
using ILLink.Shared.TypeSystemProxy;
using Mono.Cecil;

namespace ILLink.Shared.TypeSystemProxy
{
    record class ParameterProxy(MethodDefinition m, ParameterIndex i);
    record class ParameterProxyEnumerable(int i, int j, MethodDefinition m);
}

namespace Mono.Linker
{
    class LinkContext
    {
        public MethodDefinition? Resolve(MethodReference methodReference) => throw new NotImplementedException();
        public TypeDefinition? TryResolve(TypeReference typeReference) => throw new NotImplementedException();
    }

    static class BCL
    {
        public static TypeDefinition? FindPredefinedType(WellKnownType type, LinkContext context) => throw new NotImplementedException();
    }
}
