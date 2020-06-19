package com.mcres.luckyfish.angustifolia.fluoxetine.util.info;

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Objects;

public class MethodDescriptor {
	private final int modifier;
	private final String name;
	private final String descriptor;
	private final String signature;
	private final String[] exceptions;

	private final List<Parameter> parameters = new LinkedList<>();

	public MethodDescriptor(int modifier, String name, String descriptor, String signature, String[] exceptions) {
		this.modifier = modifier;
		this.name = name;
		this.descriptor = descriptor;
		this.signature = signature;
		this.exceptions = exceptions;
	}

	public int getModifier() {
		return modifier;
	}

	public String getName() {
		return name;
	}

	public String getDescriptor() {
		return descriptor;
	}

	public String getSignature() {
		return signature;
	}

	public String[] getExceptions() {
		return exceptions;
	}

	public void addParameter(Parameter param) {
		this.parameters.add(param);
	}

	public List<Parameter> getParameters() {
		return Collections.unmodifiableList(this.parameters);
	}

	@Override
	public boolean equals(Object o) {
		if (this == o) return true;
		if (o == null || getClass() != o.getClass()) return false;
		MethodDescriptor that = (MethodDescriptor) o;
		return name.equals(that.name) &&
				descriptor.equals(that.descriptor) &&
				Objects.equals(signature, that.signature);
	}

	@Override
	public int hashCode() {
		return Objects.hash(name, descriptor, signature);
	}

	public static class Parameter {
		private final String name;
		private final int access;

		public Parameter(String name, int access) {
			this.name = name;
			this.access = access;
		}

		public String getName() {
			return name;
		}

		public int getAccess() {
			return access;
		}
	}
}
